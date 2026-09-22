# ESP32 E-Textiles Projects

Open one project folder at a time in PlatformIO:

- [Pebble light controller](pebble-controller/README.md): the original Wi-Fi color controller and class provisioning tools.
- [Programming workshop](programming-workshop/README.md): code-only LED activities. Start with the [activity guide](programming-workshop/ACTIVITY.md).

From this repository root:

```sh
pio run --project-dir pebble-controller
pio run --project-dir programming-workshop
```

For provisioning, first change into `pebble-controller`, then follow its README. For workshop editing and uploads, open `programming-workshop` and edit `src/main.cpp`.

The Git repository remains at this root. Existing root `.pio` and generated `.vscode` files are stale local caches, not project configuration; build and debug inside the chosen project folder. No hardware upload is required to build either project.
