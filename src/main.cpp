#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include "iro_asset.h"

constexpr char AP_SSID[] = "Pebble-Test";
constexpr char AP_PASSWORD[] = "pebble123";
constexpr int NUM_LEDS = 60;
CRGB leds[NUM_LEDS];
CRGB color = CRGB::Red;
uint8_t brightness = 20;
bool enabled = false;
WebServer server(80);
bool apReady = false;

const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>Pebble · 灯光控制</title><style>
*{box-sizing:border-box}body{margin:0;background:#f4f3ef;color:#17251f;font:16px system-ui,-apple-system,sans-serif}main{max-width:640px;margin:auto;padding:24px 14px 130px}header{display:flex;align-items:center;justify-content:space-between;gap:12px}h1{font-size:26px;margin:0}p{line-height:1.5}.muted{color:#55635b;font-size:14px}#status{font-size:13px;white-space:nowrap}#preview{height:46px;border:1px solid #adb8b0;border-radius:16px;margin-top:20px}.readout{display:flex;justify-content:space-between;align-items:center;margin:8px 0 14px}#hex{font:600 18px ui-monospace,monospace}button,input{font:inherit}button{cursor:pointer;touch-action:manipulation}button:focus-visible,input:focus-visible{outline:3px solid #176649;outline-offset:3px}#quick{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:6px;margin:12px 0 20px}.quick{min-height:48px;border:1px solid #bbc5be;background:white;border-radius:10px;color:#17251f}.dot{display:block;width:16px;height:16px;border:1px solid #a5aaa6;border-radius:50%;margin:0 auto 3px}#wheel{width:min(100%,340px);margin:12px auto 18px;touch-action:none;user-select:none}#wheel:focus-visible{outline:3px solid #176649;outline-offset:4px;border-radius:50%}.quick[aria-pressed=true]{box-shadow:inset 0 0 0 2px #17251f}input[type=range]{width:100%;height:44px;accent-color:#176649}.slider-label{display:flex;justify-content:space-between;gap:8px}.footer{position:fixed;bottom:0;left:0;right:0;padding:12px 14px calc(12px + env(safe-area-inset-bottom));background:#f4f3eff2;border-top:1px solid #d7ddd7}.footer button{display:block;width:100%;max-width:612px;margin:auto;min-height:54px;border:0;border-radius:12px;background:#17251f;color:#fff;font-weight:650}h2{font-size:16px;margin:0}#light{font-size:14px}#status[data-error=true]{color:#a32c25}
</style></head><body><main>
<header><h1>Pebble</h1><span id="status" role="status" aria-live="polite">连接中</span></header>
<p class="muted">60 颗 · 全部同色 · 拖动即变色</p>
<div id="preview" aria-label="选定颜色预览"></div><div class="readout"><span id="hex">#FF0000</span><span id="light">已熄灭</span></div>
<div id="wheel" tabindex="0" role="group" aria-label="连续色轮：左右键调整色相，上下键调整饱和度" aria-describedby="wheel-help"></div>
<p id="wheel-help" class="muted">绕圈换颜色，向中心变浅。按住拖动即可变色。</p>
<label class="slider-label" for="brightness"><span>亮度 <strong id="level">20</strong></span><span class="muted">上限 20 / 255</span></label><input id="brightness" type="range" min="0" max="20" value="20">
<div id="quick" aria-label="快捷颜色"></div>
</main><div class="footer"><button id="off" type="button">全部熄灭</button></div>
<script src="/iro.min.js"></script>
<script>
const $=id=>document.getElementById(id);
let desired={color:'#ff0000',brightness:20,enabled:false};
let wheel=null,dragging=false,lastHue=0;
let busy=false,pending=null,offPending=false,timer=null,lastSent=-Infinity,revision=0;
function status(text,error=false){$('status').textContent=text;$('status').dataset.error=error;}
function render(){
 $('preview').style.backgroundColor=desired.color;$('hex').textContent=desired.color.toUpperCase();
 syncWheel();$('brightness').value=desired.brightness;$('level').textContent=desired.brightness;
 $('light').textContent=desired.brightness===0?'亮度为 0':(!desired.enabled||desired.color==='#000000'?'已熄灭':'同色常亮');
 document.querySelectorAll('[data-color]').forEach(b=>b.setAttribute('aria-pressed',String(b.dataset.color===desired.color)));
}
// 程序同步只更新选点，不触发用户输入事件；灰色保留上一次色相。
function syncWheel(){
 if(!wheel||dragging)return;
 const next=new iro.Color(desired.color).hsv;
 if(next.s===0||next.v===0)next.h=lastHue;else lastHue=next.h;
 next.v=100;wheel.color.hsv=next;
}
function choose(value){desired.color=value.toLowerCase();desired.enabled=true;revision++;render();pending={...pending,color:desired.color,brightness:desired.brightness};pump();}
function wheelInput(){lastHue=wheel.color.hue;choose(wheel.color.hexString);}
wheel=new iro.ColorPicker('#wheel',{
 width:Math.min(340,$('wheel').clientWidth),color:'#ff0000',
 handleRadius:11,borderWidth:1,borderColor:'#87968b',wheelLightness:false,
 layout:[{component:iro.ui.Wheel}]
});
wheel.on('input:start',()=>{dragging=true;wheelInput();});
wheel.on('input:move',wheelInput);
wheel.on('input:end',()=>{wheelInput();dragging=false;});
window.addEventListener('resize',()=>wheel.resize(Math.min(340,$('wheel').clientWidth)));
$('wheel').addEventListener('keydown',e=>{
 if(!['ArrowLeft','ArrowRight','ArrowUp','ArrowDown'].includes(e.key))return;
 e.preventDefault();const step=e.shiftKey?10:2;
 const h=wheel.color.hsv;
 if(e.key==='ArrowLeft')h.h=(h.h-step+360)%360;
 if(e.key==='ArrowRight')h.h=(h.h+step)%360;
 if(e.key==='ArrowUp')h.s=Math.min(100,h.s+step);
 if(e.key==='ArrowDown')h.s=Math.max(0,h.s-step);
 wheel.color.hsv=h;wheelInput();
});
[['红','#ff0000'],['绿','#00ff00'],['蓝','#0000ff'],['白','#ffffff']].forEach(([label,value])=>{
 const b=document.createElement('button');b.type='button';b.dataset.color=value;b.className='quick';
 const dot=document.createElement('span');dot.className='dot';dot.style.backgroundColor=value;
 b.append(dot,document.createTextNode(label));b.onclick=()=>choose(value);$('quick').append(b);
});
$('brightness').addEventListener('input',e=>{desired.brightness=Number(e.target.value);revision++;render();pending={...pending,brightness:desired.brightness};pump();});
$('off').onclick=()=>{desired.enabled=false;revision++;render();pending=null;offPending=true;clearTimeout(timer);pump();};
async function request(path,values){const controller=new AbortController();const timeout=setTimeout(()=>controller.abort(),3000);
 try{const options={signal:controller.signal,cache:'no-store'};if(values!==undefined){options.method='POST';options.body=new URLSearchParams(values);}const response=await fetch(path,options);if(!response.ok)throw Error('HTTP '+response.status);return await response.json();}finally{clearTimeout(timeout);}}
async function pump(){
 clearTimeout(timer);if(busy||(!offPending&&!pending))return;
 const wait=80-(performance.now()-lastSent);if(!offPending&&wait>0){timer=setTimeout(pump,wait);return;}
 const off=offPending;const payload=off?{}:pending;if(off)offPending=false;else pending=null;
 busy=true;lastSent=performance.now();const sentRevision=revision;status('发送中');
 try{const result=await request(off?'/api/off':'/api/set',payload);
 if(sentRevision===revision){desired={color:result.color,brightness:result.brightness,enabled:result.enabled};render();}status('已发送');
 }catch(e){pending=null;status('连接中断',true);$('light').textContent='结果未确认';}
 finally{busy=false;pump();}
}
async function init(){render();busy=true;const initialRevision=revision;
 try{const result=await request('/api/state');if(revision===initialRevision){desired={color:result.color,brightness:result.brightness,enabled:result.enabled};render();}status('已连接');}
 catch(e){status('连接中断',true);}finally{busy=false;pump();}}
init();
</script></body></html>)HTML";

void sendState() {
    char value[8];
    snprintf(value, sizeof(value), "#%02x%02x%02x", color.r, color.g, color.b);
    String body = "{\"color\":\"" + String(value) + "\",\"brightness\":" + String(brightness);
    body += ",\"enabled\":";
    body += enabled ? "true" : "false";
    body += ",\"isOn\":";
    body += (enabled && brightness > 0 && (color.r || color.g || color.b)) ? "true}" : "false}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", body);
}

void applyLight() {
    FastLED.setBrightness(brightness);
    fill_solid(leds, NUM_LEDS, enabled ? color : CRGB::Black);
    FastLED.show();
}

void setLight() {
    CRGB nextColor = color;
    int nextBrightness = brightness;
    bool hasColor = server.hasArg("color");
    bool valid = hasColor || server.hasArg("brightness");
    if (hasColor) {
        String value = server.arg("color");
        valid = valid && value.length() == 7 && value[0] == '#';
        for (unsigned i = 1; i < value.length(); ++i) valid = valid && isxdigit(static_cast<unsigned char>(value[i]));
        if (valid) nextColor = CRGB(static_cast<uint32_t>(strtoul(value.c_str() + 1, nullptr, 16)));
    }
    if (server.hasArg("brightness")) {
        String value = server.arg("brightness");
        valid = valid && !value.isEmpty() && value.length() <= 3;
        for (unsigned i = 0; i < value.length(); ++i) valid = valid && isdigit(static_cast<unsigned char>(value[i]));
        if (valid) nextBrightness = constrain(value.toInt(), 0L, 20L);
    }
    if (!valid) { server.send(400, "application/json", "{\"error\":\"invalid value\"}"); return; }
    color = nextColor;
    brightness = nextBrightness;
    if (hasColor) enabled = true;
    applyLight();
    sendState();
}

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<WS2812B, D10, GRB>(leds, NUM_LEDS);
    applyLight();
    WiFi.mode(WIFI_AP);
    IPAddress ip(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    if (!WiFi.softAPConfig(ip, ip, subnet) || !WiFi.softAP(AP_SSID, AP_PASSWORD)) {
        Serial.println("ERROR: Wi-Fi AP startup failed. Press RESET to retry.");
        return;
    }
    server.on("/", HTTP_GET, []() {
        server.sendHeader("Cache-Control", "no-store");
        server.send_P(200, "text/html; charset=utf-8", PAGE);
    });
    server.on("/iro.min.js", HTTP_GET, []() {
        server.sendHeader("Content-Encoding", "gzip");
        server.sendHeader("Cache-Control", "public, max-age=86400");
        server.send_P(200, "application/javascript", reinterpret_cast<const char*>(IRO_JS_GZ), sizeof(IRO_JS_GZ));
    });
    server.on("/api/state", HTTP_GET, sendState);
    server.on("/api/set", HTTP_POST, setLight);
    server.on("/api/off", HTTP_POST, []() { enabled = false; applyLight(); sendState(); });
    server.begin();
    apReady = true;
    Serial.println("Wi-Fi AP ready: Pebble-Test");
    Serial.println("Open http://192.168.4.1");
}

void loop() {
    if (apReady) server.handleClient();
    delay(2);
}
