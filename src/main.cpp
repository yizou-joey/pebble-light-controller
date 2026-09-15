#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "iro_asset.h"

// Strand lengths differ from student to student, so the live count is set from the
// web page and remembered in flash; the buffer just has to be big enough for anyone.
constexpr int MAX_LEDS = 100;
// The LEDs are powered from the XIAO's 5V pin, which is just USB power passed through:
// a laptop port gives roughly 500-900 mA. 60 LEDs at full white would want ~3600 mA,
// so FastLED is given this budget and dims the output as needed to stay inside it.
// Raise it only when every board runs from a 2 A wall charger.
constexpr uint32_t LED_MILLIAMP_BUDGET = 900;
char apSsid[16] = "Pebble";  // per-board name, filled from the chip MAC in setup()
CRGB leds[MAX_LEDS];
int ledCount = 60;
CRGB color = CRGB::Red;
uint8_t brightness = 20;
bool enabled = false;
WebServer server(80);
Preferences prefs;
bool apReady = false;

const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>Pebble · 灯光控制</title><style>
*{box-sizing:border-box}body{margin:0;background:#f4f3ef;color:#17251f;font:16px system-ui,-apple-system,sans-serif}main{max-width:640px;margin:auto;padding:24px 14px 130px}header{display:flex;align-items:center;justify-content:space-between;gap:12px}h1{font-size:26px;margin:0}p{line-height:1.5}.muted{color:#55635b;font-size:14px}#status{font-size:13px;white-space:nowrap}#preview{height:46px;border:1px solid #adb8b0;border-radius:16px;margin-top:20px}.readout{display:flex;justify-content:space-between;align-items:center;margin:8px 0 14px}#hex{font:600 18px ui-monospace,monospace}button,input{font:inherit}button{cursor:pointer;touch-action:manipulation}button:focus-visible,input:focus-visible{outline:3px solid #176649;outline-offset:3px}#quick{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:6px;margin:12px 0 20px}.quick{min-height:48px;border:1px solid #bbc5be;background:white;border-radius:10px;color:#17251f}.dot{display:block;width:16px;height:16px;border:1px solid #a5aaa6;border-radius:50%;margin:0 auto 3px}#wheel{width:min(100%,340px);margin:12px auto 18px;touch-action:none;user-select:none}#wheel:focus-visible{outline:3px solid #176649;outline-offset:4px;border-radius:50%}.quick[aria-pressed=true]{box-shadow:inset 0 0 0 2px #17251f}input[type=range]{width:100%;height:44px;accent-color:#176649}.slider-label{display:flex;justify-content:space-between;gap:8px}.footer{position:fixed;bottom:0;left:0;right:0;padding:12px 14px calc(12px + env(safe-area-inset-bottom));background:#f4f3eff2;border-top:1px solid #d7ddd7}.footer button{display:block;width:100%;max-width:612px;margin:auto;min-height:54px;border:0;border-radius:12px;background:#17251f;color:#fff;font-weight:650}h2{font-size:16px;margin:0}#light{font-size:14px}#status[data-error=true]{color:#a32c25}
</style></head><body><main>
<header><h1>Pebble</h1><span id="status" role="status" aria-live="polite">连接中</span></header>
<p class="muted">全部同色 · 拖动即变色</p>
<div id="preview" aria-label="选定颜色预览"></div><div class="readout"><span id="hex">#FF0000</span><span id="light">已熄灭</span></div>
<div id="wheel" tabindex="0" role="group" aria-label="连续色轮：左右键调整色相，上下键调整饱和度" aria-describedby="wheel-help"></div>
<p id="wheel-help" class="muted">绕圈换颜色，向中心变浅。按住拖动即可变色。</p>
<label class="slider-label" for="brightness"><span>亮度 <strong id="level">20</strong></span><span class="muted">最大 255 · 电源自动限流</span></label><input id="brightness" type="range" min="0" max="255" value="20">
<div id="quick" aria-label="快捷颜色"></div>
<label class="slider-label" for="count" style="margin-top:8px"><span>灯珠数量</span><span class="muted">1–100 · 断电后保留</span></label>
<input id="count" type="number" min="1" max="100" value="60" inputmode="numeric" style="width:100%;min-height:44px;border:1px solid #bbc5be;border-radius:10px;padding:0 12px;background:#fff">
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
$('count').addEventListener('change',async e=>{
 let v=Math.max(1,Math.min(100,Math.round(Number(e.target.value))||1));e.target.value=v;
 try{const result=await request('/api/set',{count:v});$('count').value=result.count;status('已发送');}
 catch(err){status('连接中断',true);}
});
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
 try{const result=await request('/api/state');if(result.board){document.querySelector('h1').textContent=result.board;document.title=result.board+' · 灯光控制';}if(result.count)$('count').value=result.count;if(revision===initialRevision){desired={color:result.color,brightness:result.brightness,enabled:result.enabled};render();}status('已连接');}
 catch(e){status('连接中断',true);}finally{busy=false;pump();}}
init();
</script></body></html>)HTML";

void sendState() {
    char value[8];
    snprintf(value, sizeof(value), "#%02x%02x%02x", color.r, color.g, color.b);
    String body = "{\"board\":\"" + String(apSsid) + "\",\"color\":\"" + String(value) + "\",\"brightness\":" + String(brightness);
    body += ",\"enabled\":";
    body += enabled ? "true" : "false";
    body += ",\"count\":" + String(ledCount);
    body += ",\"isOn\":";
    body += (enabled && brightness > 0 && (color.r || color.g || color.b)) ? "true}" : "false}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", body);
}

void applyLight() {
    FastLED.setBrightness(brightness);
    fill_solid(leds, MAX_LEDS, CRGB::Black);
    if (enabled) fill_solid(leds, ledCount, color);
    FastLED.show();
}

void setLight() {
    CRGB nextColor = color;
    int nextBrightness = brightness;
    int nextCount = ledCount;
    bool hasColor = server.hasArg("color");
    bool valid = hasColor || server.hasArg("brightness") || server.hasArg("count");
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
        if (valid) nextBrightness = constrain(value.toInt(), 0L, 255L);
    }
    if (server.hasArg("count")) {
        String value = server.arg("count");
        valid = valid && !value.isEmpty() && value.length() <= 3;
        for (unsigned i = 0; i < value.length(); ++i) valid = valid && isdigit(static_cast<unsigned char>(value[i]));
        if (valid) nextCount = constrain(value.toInt(), 1L, static_cast<long>(MAX_LEDS));
    }
    if (!valid) { server.send(400, "application/json", "{\"error\":\"invalid value\"}"); return; }
    color = nextColor;
    brightness = nextBrightness;
    if (nextCount != ledCount) {
        ledCount = nextCount;
        prefs.putInt("count", ledCount);
    }
    if (hasColor) enabled = true;
    applyLight();
    sendState();
}

void setup() {
    Serial.begin(115200);
    prefs.begin("pebble", false);
    ledCount = constrain(prefs.getInt("count", ledCount), 1, MAX_LEDS);
    FastLED.addLeds<WS2812B, D10, GRB>(leds, MAX_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MILLIAMP_BUDGET);
    applyLight();
    uint64_t mac = ESP.getEfuseMac();
    // Suffix = last two bytes of the MAC printed on the chip, so each board gets its own network.
    uint16_t suffix = static_cast<uint16_t>(((mac >> 32) & 0xff) << 8 | ((mac >> 40) & 0xff));
    // A human letter (A, B, C...) set once during provisioning rides along after the hex;
    // the hex guarantees uniqueness even if two boards ever get taped with the same letter.
    String letter = prefs.getString("letter", "");
    if (letter.length() == 1 && isupper(static_cast<unsigned char>(letter[0])))
        snprintf(apSsid, sizeof(apSsid), "Pebble-%04X-%c", suffix, letter[0]);
    else
        snprintf(apSsid, sizeof(apSsid), "Pebble-%04X", suffix);
    // Ten boards in one room: spread them over channels 1 / 6 / 11 instead of all defaulting to 1.
    int channel = 1 + 5 * static_cast<int>(mac % 3);
    WiFi.mode(WIFI_AP);
    IPAddress ip(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    if (!WiFi.softAPConfig(ip, ip, subnet) || !WiFi.softAP(apSsid, nullptr, channel)) {
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
    Serial.print("Wi-Fi AP ready (open network): ");
    Serial.println(apSsid);
    Serial.println("Open http://192.168.4.1");
}

// Provisioning-time serial commands: "letter X" saves the board letter and reboots
// so the network name picks it up; "letter ?" reports the current one.
void handleSerial() {
    static String line;
    while (Serial.available()) {
        char c = Serial.read();
        if (c != '\n' && c != '\r') { if (line.length() < 32) line += c; continue; }
        if (line.startsWith("letter ")) {
            String arg = line.substring(7);
            arg.trim();
            if (arg == "?") {
                Serial.printf("letter=%s ssid=%s\n", prefs.getString("letter", "(none)").c_str(), apSsid);
            } else if (arg.length() == 1 && isupper(static_cast<unsigned char>(arg[0]))) {
                prefs.putString("letter", arg);
                Serial.printf("letter=%s saved, rebooting\n", arg.c_str());
                delay(100);
                ESP.restart();
            } else {
                Serial.println("usage: letter A..Z | letter ?");
            }
        }
        line = "";
    }
}

void loop() {
    if (apReady) server.handleClient();
    handleSerial();
    delay(2);
}
