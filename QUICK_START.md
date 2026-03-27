# 🚀 Quick Start Guide

Get your ESP32 authenticating with SRAM PUF in 30 minutes!

---

## Terminal Setup

**This guide uses multiple terminal windows for clarity.**

I recommend opening **3 terminals** and labeling them:

```
┌─────────────────────────────────────────────────────────┐
│ Terminal 1: Server Management                           │
│ → Start/stop Docker containers                          │
│ → Restart servers after config changes                  │
│ → Always in: server/                                    │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│ Terminal 2: Server Logs (Optional)                      │
│ → Monitor uploads and authentication in real-time       │
│ → See what's happening on the server                    │
│ → Always in: server/                                    │
└─────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│ Terminal 3: Tools & Analysis                            │
│ → Generate PUF challenges                               │
│ → Download measurements                                 │
│ → Run analysis scripts                                  │
└─────────────────────────────────────────────────────────┘
```

**All commands show full paths from repository root!**

---

## Prerequisites

### Hardware
- ESP32 development board (WROOM-32, WROVER-B, or ESP32-S)
- USB cable
- WiFi network (2.4 GHz) — **see Network Setup section below before starting**

### Software

**Required:**
- [Arduino IDE](https://www.arduino.cc/en/software) with [ESP32 support](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
- [Python 3.7+](https://www.python.org/downloads/) (for PUF challenge generation)

**Server (choose one):**
- **Option A:** [Docker Desktop](https://www.docker.com/products/docker-desktop/) (recommended - includes everything)
- **Option B:** [Node.js](https://nodejs.org/) v14+ (if you prefer manual setup)

**Note:** If you use Docker, you don't need Node.js installed!

---

## ⚠️ Network Setup (Read This First!)

Getting the ESP32 and your server on the **same network** is the most common source of frustration. Read this section carefully before doing anything else.

### The Core Requirement

Your ESP32 and your computer must be on the **same subnet** — i.e., they must get IP addresses in the same range (e.g., both on `192.168.1.x`). If they're on different subnets, they cannot communicate even if they're both connected to the same hotspot.

### Best Option: Home/Office Router

Connect both your ESP32 and computer to a regular 2.4 GHz WiFi router. This is the simplest setup — both devices will get IPs on the same subnet automatically.

**Find your computer's IP:**
- macOS/Linux: `ifconfig | grep "inet " | grep -v 127.0.0.1`
- Windows: `ipconfig | findstr "IPv4"`

Use that IP as your server URL: `http://YOUR_IP:3000/api/sram`

### University WiFi

University networks typically block device-to-device communication and require browser-based login. **ESP32 cannot connect to these networks.** You'll need an alternative — see iPhone Hotspot below.

### iPhone Hotspot (Common Pitfall)

iPhone hotspots work but have a tricky subnet issue when **Maximize Compatibility** is enabled.

**The problem:** When Maximize Compatibility is ON (required for ESP32's 2.4 GHz), iPhone assigns different subnets to different types of clients:
- WiFi clients (ESP32) → `172.20.10.x`
- "Tethered clients" (your Mac and other Apple Devices) → `192.0.0.x`

These subnets cannot reach each other, so uploads will fail even though both devices appear connected.

**Fix — force your Mac onto the same subnet as the ESP32:**

1. Enable hotspot on iPhone with **Maximize Compatibility ON**
2. Connect your Mac to the hotspot via WiFi
3. Go to **System Settings → WiFi → [your hotspot] → Details → TCP/IP**
4. Change "Configure IPv4" to **Manual**
5. Set IP to `<first seven digits of your ESP32's IP>.x`, different from your ESP32's IP, Subnet Mask to `255.255.255.240`
6. Run `ifconfig | grep "inet "` — you should now see the assigned IP pop up
7. Use the assigbed IP as your server IP in all URLs

**Verify they can talk:**
```bash
# Should return "Cannot GET /api/sram" (which means it's reachable!)
curl http://172.20.10.3:3000/api/sram
```

> **Note:** If you have a VPN or iCloud Private Relay active, disable it. These create extra network tunnels (visible as `utun` interfaces) that can interfere with local network routing.

### Android Hotspot

Android hotspots do **not** have the subnet splitting issue that iPhones have. Both your computer and ESP32 will land on the same subnet automatically. Recommended over iPhone if available. Untested, but per an internet search, this should be the case.

### Confirming Your Setup Works

Before flashing any sketch, confirm connectivity:

1. Start your server (Step 1 below)
2. From your Mac, run:
   ```bash
   curl -u "esp:password" http://YOUR_SERVER_IP:3000/api/sram
   ```
   You should see `Cannot GET /api/sram` — this means the server is reachable (it only accepts POST).
3. Only then flash the ESP32 sketch

---

## Step 1: Setup Measurement Server

Choose **one** deployment method:

### Option A: Docker (Recommended)

**Terminal 1 (Server Management):**
```bash
# Navigate to server directory (from repository root)
cd server

# Start servers in background
docker compose up -d --build

# Note: Older Docker versions use 'docker-compose' (with hyphen)
# If you get "command not found", try: docker-compose ps
```

**Verify servers are running:**
```bash
# In Terminal 1 (still in server/)
docker compose ps
```

You should see both services with status "Up":
```
NAME                   IMAGE                       COMMAND                  SERVICE              CREATED         STATUS                            PORTS
sram-puf-auth          server-auth-server          "docker-entrypoint.s…"   auth-server          9 seconds ago   Up 8 seconds (health: starting)   0.0.0.0:8080->8080/tcp
sram-puf-measurement   server-measurement-server   "docker-entrypoint.s…"   measurement-server   9 seconds ago   Up 8 seconds (health: starting)   0.0.0.0:3000->3000/tcp
```

Both servers are now running:
- Authentication Server: http://localhost:8080
- Measurement Server: http://localhost:3000

**Default credentials (for testing):**
- Username: `esp`
- Password: `password`

To change credentials, edit `docker-compose.yml` and restart:
```bash
# Edit environment variables in docker-compose.yml
nano docker-compose.yml

# Look for ESP_USERNAME and ESP_PASSWORD, then:
docker compose down
docker compose up -d
```

**Useful commands (Terminal 1):**
```bash
# Still in server/
docker compose ps                    # Check status
docker compose logs -f               # View all logs
docker compose down                  # Stop servers
docker compose restart auth-server   # Restart one service
```

**Done!** Skip to Step 2.

---

### Option B: Manual Setup

**Terminal 1 (Measurement Server):**
```bash
# From repository root
cd server/measurement-server

# Create .env file from template
cp env.example .env

# Optional: Edit credentials
# nano .env

# Install and start
npm install
npm start
```

Server runs on http://localhost:3000

**Default credentials:** `esp` / `password` (see `.env` file)

**Terminal 2 (Authentication Server):**
```bash
# From repository root (new terminal!)
cd server/auth-server
npm install
npm start
```

Server runs on http://localhost:8080

---

## Step 2: Collect SRAM Measurements

### Configure ESP32

1. Open `hardware/sram_reader_with_upload.ino` in Arduino IDE

2. Update WiFi credentials:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   ```

   > **Tip:** Avoid special characters (apostrophes, spaces) in your hotspot name. Use something simple like `esp32test`. Special characters in SSIDs can cause connection failures on ESP32.

3. Update server URL (replace with your computer's IP):
   ```cpp
   #define MEASUREMENT_SERVER_URL "http://192.168.1.XXX:3000/api/sram"
   ```

   Find your IP:
   - macOS/Linux: `ifconfig | grep "inet " | grep -v 127.0.0.1`
   - Windows: `ipconfig | findstr "IPv4"`

   > **Tip:** If using iPhone hotspot with the manual IP fix described above, use `172.20.10.3` (or whatever IP you set manually).

4. Set ESP ID (use 1 for your first device):
   ```cpp
   #define ESP_DEVICE_ID 1
   ```

5. **Optional:** Use 128 bytes for faster collection:
   ```cpp
   #define SRAM_READ_SIZE 128  // Sufficient for authentication
   ```

### ESP32 WiFi Setup — Correct Init Order

The order of WiFi initialization calls matters. Use this exact sequence to avoid stale credential issues:

```cpp
WiFi.disconnect(true, true);  // clear old state
delay(1000);
WiFi.persistent(false);       // don't cache credentials to flash
WiFi.mode(WIFI_STA);
delay(100);
WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
```

Also increase the Serial initialization delay so print statements appear correctly:

```cpp
Serial.begin(115200);
delay(1000);  // use 1000ms, not 100ms
```

### Debugging WiFi Status Codes

If WiFi fails to connect, print the status code during the connection loop:

```cpp
while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print(".");
    Serial.print(WiFi.status());
    attempts++;
}
```

| Code | Meaning | Fix |
|------|---------|-----|
| `1` | No SSID found | Hotspot not visible or wrong name |
| `4` | Connect failed | Wrong password |
| `6` | Wrong password | Check password, try simpler one |
| `255` | Idle / not started | Check init order above |

A common pattern is `6` then `1` — this means the password was rejected and then the network disappeared. The most likely cause is **5 GHz only mode**. On iPhone: **Settings → Personal Hotspot → Maximize Compatibility → ON** to force 2.4 GHz.

### Upload & Collect

1. Select your ESP32 board: **Tools → Board → ESP32 Dev Module**
2. Upload the sketch
3. Open Serial Monitor (115200 baud)

You should see:
```
WiFi connected!
Uploading measurement...
✓ Upload successful!
```

**Monitor uploads live (if using Docker):**

**Terminal 2 (Server Logs):**
```bash
# From repository root
cd server
docker compose logs measurement-server -f
```

You'll see each upload in real-time:
```
sram-puf-measurement  | [2025-10-08T11:42:44.795Z] POST /api/sram
sram-puf-measurement  |   [DATA] ESP ID: 1
sram-puf-measurement  |   [DATA] Data Length: 256 chars
sram-puf-measurement  |   [SUCCESS] Stored measurement ID: 1
sram-puf-measurement  | [2025-10-08T11:42:56.795Z] POST /api/sram
sram-puf-measurement  |   [DATA] ESP ID: 1
sram-puf-measurement  |   [DATA] Data Length: 256 chars
sram-puf-measurement  |   [SUCCESS] Stored measurement ID: 2
```

Note: Press `Ctrl+C` if you want to stop viewing logs.

### Power Cycle for Measurements

**CRITICAL:** SRAM PUF only changes on complete power loss!

```
❌ Reset button → Same SRAM values
✅ Disconnect USB → Wait 2 seconds → Reconnect → New values
```

**Collect measurements:**
1. Disconnect USB cable
2. Wait 2 seconds
3. Reconnect USB
4. Wait for upload confirmation
5. Repeat as many times as you want (minimum 30 recommended)

**Why 30+?** More measurements = better stable bit identification = higher authentication success rate. You can collect 50, 100, or more for even better results.

See the success rate table in Step 5 for detailed statistics.

---

## Step 3: Download & Generate PUF Challenge

### Download Measurements

**Terminal 3 (Tools):**
```bash
# Download measurements for ESP ID 1
curl -u "esp:password" http://localhost:3000/api/export/1 -o measurements.txt

# Or with your network IP (if server is remote)
# curl -u "esp:password" http://172.20.10.3:3000/api/export/1 -o measurements.txt
```

**Credentials:** See `server/measurement-server/env.example` (default: `esp` / `password`)

### Generate PUF Challenge & API Token

**Terminal 3:**
```bash
# Navigate to tools
cd tools/puf-challenge-generator

# Generate challenge from measurements
python pufchallenge.py -i ../../measurements.txt
```

**Output:**
```
[ESP32]  PUF Challenge: f3f3dfffefbabffff3fbff7fb6efbeff...
[Server] API-Token:     9b6fa081f05bc4b197f4ff7e79f01...
```

**Copy both values!** You'll need them in the next steps.

---

## Step 4: Configure Authentication

### Configure Server

**Terminal 3 (Tools):**
Edit config file (use your preferred editor):
```bash
# From repository root
nano server/auth-server/config.js
```

The file structure must look exactly like this — pay close attention to closing braces and commas:

```javascript
module.exports = {
    port: process.env.PORT || 8080,

    apiKeys: {
        'YOUR_API_TOKEN_HERE': {
            deviceId: 'ESP32_001',
            description: 'My first ESP32',
            registered: '2024-01-15'
        },
        // Add more devices here, each entry needs a trailing comma except the last
    },  // <-- this comma is required! closes apiKeys, not the whole export

    security: {
        debugEndpoints: true,
        logAttempts: true
    }
};
```

> **Common mistake:** Placing the `security` block *inside* `apiKeys` instead of as a sibling. If auth-server keeps restarting, check `docker logs sram-puf-auth` — a `SyntaxError: Unexpected token` means a missing comma or misplaced brace in config.js.

**Apply changes:**

**Terminal 1 (Server Management):**
```bash
# In ./server/
docker compose restart auth-server
```

**Note:** The config.js file is mounted as a volume in Docker, so you can edit it directly without rebuilding the container!

### Configure ESP32

1. Open `hardware/esp32_authenticate_simple.ino`

2. Update configuration:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourWiFiPassword"
   #define AUTH_SERVER_URL "http://YOUR_SERVER_IP:8080/"

   // Paste your PUF Challenge here:
   #define PUF_CHALLENGE "f3f3dfffefbabffff3fbff7fb6efbeff..."
   ```

3. Upload to ESP32

---

## Step 5: Test Authentication

Open Serial Monitor (115200 baud). You should see:

```
========================================
ESP32 RTC SLOW Memory PUF Authentication
========================================

[1/4] Reading RTC SLOW Memory...
      Address: 0x50000000
      Size: 128 bytes
      Expected stability: ~94% (thesis)

[2/4] Extracting PUF bits...
      Challenge: ff2ee7f75f7effeffefbfffbe7ffe5fe
      Extracted bits: 10101011100110001011101000100101011111101011110011000001101011000100011000100000000000001010110101110001100
      Bit count: 107

[3/4] Generating PBKDF2 key...
      Iterations: 10000
      Derived Key: 260f1105f729c7afcd71b62eacb5c0b94fcca4c16f6be36c4b2a297a975d358a

[4/4] Connecting to WiFi...
      SSID: [censored]
......
      Connected!
      IP: 192.168.1.XXX

[AUTH] Attempting authentication...
      Server: http://192.168.1.XXX:8080/
[INFO] Bearer Token: Bearer 260f1105f729c7afcd71b62eacb5c0b94fcca4c16f6be36c4b2a297a975d358a
[INFO] HTTP Response Code: 200
[INFO] Response: {"authenticated":true,"deviceId":"ESP32_001","description":"Test Device 1","message":"ESP32_001 authenticated successfully","timestamp":"2025-10-08T11:56:49.581Z"}

✓ AUTHENTICATION SUCCESSFUL!

========================================
Done. Reset ESP32 to try again.
========================================
```

**Monitor authentication live (if using Docker):**

**Terminal 2 (Server Logs):**
```bash
# From repository root
cd server
docker compose logs auth-server -f
```

You'll see each authentication attempt:
```
sram-puf-auth  | [2025-10-08T11:51:22.641Z] GET / from 192.168.65.1
sram-puf-auth  |   ✅ SUCCESS: ESP32_001 authenticated
sram-puf-auth  |   ├─ Description: Test Device 1
sram-puf-auth  |   ├─ Token: 260f1105...975d358a
sram-puf-auth  |   └─ Total successful: 1
```

Press `Ctrl+C` to stop viewing logs.

---

**Success!** Your ESP32 is authenticating using SRAM PUF!

---

## Understanding Authentication Success Rates

The number of measurements you collect directly affects authentication reliability:

| Measurements Collected | Authentication Success Rate | Note |
|------------------------|----------------------------|------|
| 5 measurements | ~50% | Not recommended |
| 10 measurements | ~60% | Minimum for testing |
| 30 measurements | 100% | Recommended (tested with 10 auth attempts) |

**Key takeaway:** More measurements = better stable bit identification = higher success rate.

**Recommendation:** Collect at least 30 measurements for reliable authentication. More measurements (50-100+) will improve reliability further.

If authentication fails occasionally, collect more measurements and regenerate the PUF challenge.

### Why a Different ESP32 Cannot Authenticate

If you flash the same sketch onto a different ESP32, authentication will fail — and that's intentional. Each ESP32 has physically unique SRAM startup values due to manufacturing variation. The PUF challenge was generated from your specific device's measurements, and the API token in `config.js` was derived from your device's SRAM bits. A different device produces different bits → different token → server rejects it. The hardware itself is the password, not the code.

---

## Troubleshooting

### Auth Server Keeps Restarting

Check the logs:
```bash
docker logs sram-puf-auth
```

A `SyntaxError` in `config.js` is the most common cause. Things to check:
- `apiKeys` must be closed with `},` (comma!) before the `security` block
- `security` must be a sibling of `apiKeys`, not nested inside it
- Every device entry except the last needs a trailing comma

### Authentication Failed

**Most Common Cause: Not Enough Measurements**
- Collect 30+ measurements instead of 10-20
- Regenerate PUF challenge with more data

**Check PUF Challenge:**
- Verify you copied the complete challenge (no truncation)
- Ensure challenge matches the measurements used

**Verify PBKDF2 Settings:**
- Iterations must be **10000** in both ESP32 and Python
- Salt must match: `"ESP32-SRAM-PUF-Auth-v1"`

**Note:** `SRAM_READ_SIZE` can be 128 or 8192 bytes - both work for authentication. Use 8192 only if you plan to run analysis scripts on the measurements.

### WiFi Connection Failed

- Verify SSID and password (case-sensitive, no special characters)
- ESP32 only supports **2.4 GHz** WiFi — on iPhone hotspot, enable **Maximize Compatibility**
- Print WiFi status codes during the loop to diagnose (see Step 2 above)
- Use the correct WiFi init order (see Step 2 above)
- Increase `Serial.begin` delay to 1000ms if prints aren't showing

### Cannot Connect to Server (connection refused / -1)

1. Confirm server is running: `docker compose ps`
2. Confirm server IP is reachable from your Mac: `curl http://YOUR_IP:3000/api/sram`
3. Check your Mac's firewall: `sudo /usr/libexec/ApplicationFirewall/socketfilterfw --setglobalstate off`
4. Confirm ESP32 and Mac are on the **same subnet** (see Network Setup section)
5. If using iPhone hotspot, follow the manual IP fix in the Network Setup section
6. Add subnet route on Mac: `sudo route -n add -net 172.20.10.0/24 192.0.0.1`

### Serial Monitor Shows Garbage

- Set baud rate to **115200**
- Select correct COM port
- Try unplugging and reconnecting ESP32

### Measurements Not Uploading

- Check server logs for errors
- Verify ESP_DEVICE_ID matches URL (`/api/export/1` → ID = 1)
- Test server endpoint: `curl http://localhost:3000/health`

---

## Next Steps

### Analyze PUF Quality

Check bit stability and randomness. Choose one method:

#### Option A: Virtual Environment (Recommended)

**Terminal 3 (Tools):**
```bash
# From repository root
cd tools/analysis

# Create and activate virtual environment
python3 -m venv venv
source venv/bin/activate          # On macOS/Linux
# venv\Scripts\activate           # On Windows

# Install dependencies
pip install -r requirements.txt

# Analyze measurements
python hamming_analysis.py -i ../../measurements.txt -n ESP32_01 -c 1

# When done, deactivate virtual environment
deactivate
```

#### Option B: Global Installation

```bash
cd tools/analysis
pip install -r requirements.txt
python hamming_analysis.py -i ../../measurements.txt -n ESP32_01 -c 1
```

**Good PUF metrics:**
- Intra-device stability: >90%
- Hamming weight: ~50%

### Test Multiple Devices

If you have multiple ESP32s:

1. Collect measurements from each (use different ESP_DEVICE_ID: 2, 3, 4...)
2. Generate separate challenges for each
3. Add all API tokens to `config.js`
4. Compare uniqueness:
   ```bash
   python tools/analysis/compare_devices.py -d measurements/
   ```

**Good uniqueness:** ~50% Hamming distance between devices

### Add Security (Production)

This PoC uses HTTP for simplicity. For production:

- **Use HTTPS/TLS** instead of HTTP
- **Add replay protection** (nonce-based challenges)
- **Implement rate limiting** on authentication server
- **Enable key rotation** mechanism

See [README.md](README.md) → Security Considerations

---

## Important Notes

- This is a **Proof-of-Concept** for learning and research
- **Not production-ready** — add security hardening before deployment
- SRAM PUF responses can vary with temperature (~10% at -20°C)
- **Power cycle required** — soft reset doesn't change SRAM values
- Minimum **30 measurements** recommended (more is better — 50-100+ for higher reliability)

---

## Learn More

- [README.md](README.md) - Complete documentation
- [STRUCTURE.md](STRUCTURE.md) - Repository organization
- [SALT_CONFIGURATION.md](SALT_CONFIGURATION.md) - Customize PBKDF2 salt
- [hardware/README.md](hardware/README.md) - ESP32-specific details
- [server/measurement-server/USAGE.md](server/measurement-server/USAGE.md) - API documentation

---

**Questions?** Open an issue on GitHub or check the troubleshooting docs!

Happy experimenting!