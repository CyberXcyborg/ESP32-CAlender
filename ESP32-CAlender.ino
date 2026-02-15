#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <pgmspace.h>
#include <WiFiManager.h> // Captive portal for WiFi credentials

// Include HTML contents
#include "login_html.h"
#include "index_html.h"
#include "admin_html.h"

// Create WebServer object on port 80
WebServer server(80);

// File paths
const char* usersFilePath = "/users.json";
const char* permissionsFilePath = "/permissions.json";
const char* indexHtmlPath = "/index.html";

// Buffer for JSON document
StaticJsonDocument<8192> jsonDoc;

// Session Management
String currentSessionId = "";
String currentUsername = ""; // Track who is logged in

// Forward declarations
bool createHtmlFile();
bool connectWiFi();

// --- Auth Helpers ---

String generateSessionId() {
    String id = "";
    for(int i=0; i<16; i++) {
        id += String(random(0, 16), HEX);
    }
    return id;
}

bool checkAuth() {
    if (server.hasHeader("Cookie")) {
        String cookie = server.header("Cookie");
        if (cookie.indexOf("ESPSESSIONID=" + currentSessionId) != -1 && currentSessionId.length() > 0) {
            return true;
        }
    }
    return false;
}

bool isAdmin() {
    return checkAuth() && currentUsername == "Admin";
}

// Helper: Get list of users that 'observer' is allowed to view
void getAccessibleUsers(String observer, JsonArray& outArray) {
    // 1. Can always see self
    outArray.add(observer);

    // 2. Load permissions file
    File file = SPIFFS.open(permissionsFilePath, FILE_READ);
    if (!file) return; // No permissions file yet

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) return;

    // 3. Check if observer has grants
    if (doc.containsKey(observer)) {
        JsonArray authorized = doc[observer];
        for (JsonVariant v : authorized) {
            String user = v.as<String>();
            // Avoid adding self duplicates
            if (user != observer) {
                outArray.add(user);
            }
        }
    }
}

// Function to initialize SPIFFS and Default Data
bool initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("ERROR: SPIFFS Mount Failed");
    return false;
  }
  
  // Create/Check users file
  File file = SPIFFS.open(usersFilePath, FILE_READ);
  StaticJsonDocument<4096> usersDoc;
  bool needsSave = false;
  
  if (!file) {
      Serial.println("Creating new users file...");
      needsSave = true;
  } else {
      DeserializationError error = deserializeJson(usersDoc, file);
      file.close();
      if (error) {
          Serial.println("Users file corrupt, resetting...");
          usersDoc.clear();
          needsSave = true;
      }
  }

  // Ensure Admin user exists
  if (!usersDoc.containsKey("Admin")) {
      Serial.println("Creating default Admin user...");
      usersDoc["Admin"] = "Admin"; // Default password
      needsSave = true;
  }

  if (needsSave) {
      file = SPIFFS.open(usersFilePath, FILE_WRITE);
      serializeJson(usersDoc, file);
      file.close();
  }

  // Ensure permissions file exists (empty object)
  if (!SPIFFS.exists(permissionsFilePath)) {
      file = SPIFFS.open(permissionsFilePath, FILE_WRITE);
      if (file) {
          file.println("{}");
          file.close();
      }
  }
  
  createHtmlFile();
  return true;
}

// --- Handlers ---

void handleLoginPage() {
    if (checkAuth()) {
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
        return;
    }
    server.send(200, "text/html", loginHtml);
}

void handleLogin() {
    if (!server.hasArg("username") || !server.hasArg("password")) {
        server.send(400, "text/plain", "Missing credentials");
        return;
    }

    String username = server.arg("username");
    String password = server.arg("password");

    File file = SPIFFS.open(usersFilePath, FILE_READ);
    if (!file) {
        server.send(500, "text/plain", "Database error");
        return;
    }

    StaticJsonDocument<4096> usersDoc;
    deserializeJson(usersDoc, file);
    file.close();

    if (usersDoc.containsKey(username)) {
        String storedPass = usersDoc[username].as<String>();
        if (storedPass == password) {
            // Success
            currentSessionId = generateSessionId();
            currentUsername = username; // Store username
            server.sendHeader("Set-Cookie", "ESPSESSIONID=" + currentSessionId + "; Path=/; Max-Age=3600");
            server.send(200, "text/plain", "Login successful");
        } else {
            server.send(401, "text/plain", "Invalid password");
        }
    } else {
        server.send(401, "text/plain", "User not found");
    }
}

void handleRegister() {
    if (!server.hasArg("username") || !server.hasArg("password")) {
        server.send(400, "text/plain", "Missing credentials");
        return;
    }

    String username = server.arg("username");
    String password = server.arg("password");

    if (username.equalsIgnoreCase("Admin")) {
        server.send(403, "text/plain", "Cannot register as Admin");
        return;
    }
    
    if (username.length() < 3 || password.length() < 4) {
        server.send(400, "text/plain", "Username/Password too short");
        return;
    }

    File file = SPIFFS.open(usersFilePath, FILE_READ);
    StaticJsonDocument<4096> usersDoc;
    if (file) {
        deserializeJson(usersDoc, file);
        file.close();
    }

    if (usersDoc.containsKey(username)) {
        server.send(409, "text/plain", "User already exists");
        return;
    }

    usersDoc[username] = password;

    file = SPIFFS.open(usersFilePath, FILE_WRITE);
    serializeJson(usersDoc, file);
    file.close();
    
    server.send(200, "text/plain", "Registered");
}

void handleLogout() {
    currentSessionId = "";
    currentUsername = "";
    server.sendHeader("Set-Cookie", "ESPSESSIONID=deleted; Path=/; Max-Age=0");
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "Logged out");
}

void handleRoot() {
    if (!checkAuth()) {
        server.sendHeader("Location", "/login");
        server.send(302, "text/plain", "Redirecting to login");
        return;
    }
    server.send(200, "text/html", indexHtml);
}

// --- Admin Handlers ---

void handleAdminPage() {
    if (!isAdmin()) {
        server.send(403, "text/plain", "Forbidden: Admin Access Only");
        return;
    }
    server.send(200, "text/html", adminHtml);
}

void handleAdminGetUsers() {
    if (!isAdmin()) {
        server.send(403, "application/json", "[]");
        return;
    }

    File file = SPIFFS.open(usersFilePath, FILE_READ);
    StaticJsonDocument<4096> usersDoc;
    deserializeJson(usersDoc, file);
    file.close();

    StaticJsonDocument<4096> listDoc;
    JsonArray array = listDoc.to<JsonArray>();

    JsonObject root = usersDoc.as<JsonObject>();
    for (JsonPair kv : root) {
        array.add(kv.key().c_str());
    }

    String response;
    serializeJson(array, response);
    server.send(200, "application/json", response);
}

void handleAdminChangePass() {
    if (!isAdmin()) {
        server.send(403, "text/plain", "Forbidden");
        return;
    }

    if (!server.hasArg("newPass")) {
        server.send(400, "text/plain", "Missing password");
        return;
    }

    String newPass = server.arg("newPass");

    File file = SPIFFS.open(usersFilePath, FILE_READ);
    StaticJsonDocument<4096> usersDoc;
    deserializeJson(usersDoc, file);
    file.close();

    usersDoc["Admin"] = newPass;

    file = SPIFFS.open(usersFilePath, FILE_WRITE);
    serializeJson(usersDoc, file);
    file.close();

    server.send(200, "text/plain", "Password updated");
}

void handleAdminDeleteUser() {
    if (!isAdmin()) {
        server.send(403, "text/plain", "Forbidden");
        return;
    }

    if (!server.hasArg("username")) {
        server.send(400, "text/plain", "Missing username");
        return;
    }

    String targetUser = server.arg("username");
    if (targetUser == "Admin") {
        server.send(400, "text/plain", "Cannot delete Admin");
        return;
    }

    File file = SPIFFS.open(usersFilePath, FILE_READ);
    StaticJsonDocument<4096> usersDoc;
    deserializeJson(usersDoc, file);
    file.close();

    usersDoc.remove(targetUser);

    file = SPIFFS.open(usersFilePath, FILE_WRITE);
    serializeJson(usersDoc, file);
    file.close();

    // --- Cleanup Permissions ---
    File permFile = SPIFFS.open(permissionsFilePath, FILE_READ);
    if (permFile) {
        StaticJsonDocument<4096> permDoc;
        DeserializationError error = deserializeJson(permDoc, permFile);
        permFile.close();

        if (!error) {
            bool changed = false;
            
            // 1. Remove permissions *held by* the deleted user
            if (permDoc.containsKey(targetUser)) {
                permDoc.remove(targetUser);
                changed = true;
            }

            // 2. Remove permissions *to view* the deleted user
            JsonObject root = permDoc.as<JsonObject>();
            for (JsonPair kv : root) {
                JsonArray grants = kv.value().as<JsonArray>();
                for (int i = 0; i < grants.size(); i++) {
                    if (grants[i].as<String>() == targetUser) {
                        grants.remove(i);
                        i--; // Adjust index after removal
                        changed = true;
                    }
                }
            }

            if (changed) {
                permFile = SPIFFS.open(permissionsFilePath, FILE_WRITE);
                serializeJson(permDoc, permFile);
                permFile.close();
            }
        }
    }
    // ---------------------------

    server.send(200, "text/plain", "User deleted");
}

// Get all permissions (Admin Only)
void handleAdminGetPermissions() {
    if (!isAdmin()) {
        server.send(403, "application/json", "{}");
        return;
    }
    File file = SPIFFS.open(permissionsFilePath, FILE_READ);
    if (file) {
        server.streamFile(file, "application/json");
        file.close();
    } else {
        server.send(200, "application/json", "{}");
    }
}

// Update permissions (Admin Only)
void handleAdminSetPermissions() {
    if (!isAdmin()) {
        server.send(403, "text/plain", "Forbidden");
        return;
    }
    if (server.hasArg("plain")) {
        File file = SPIFFS.open(permissionsFilePath, FILE_WRITE);
        file.print(server.arg("plain"));
        file.close();
        server.send(200, "text/plain", "Saved");
    } else {
        server.send(400, "text/plain", "No data");
    }
}

// Reset WiFi (Admin Only)
void handleAdminResetWiFi() {
    if (!isAdmin()) {
        server.send(403, "text/plain", "Forbidden");
        return;
    }

    server.send(200, "text/plain", "WiFi Settings Cleared! Restarting...");
    delay(500);

    Serial.println("\nResetting WiFi Manager settings via Admin Panel...");
    WiFiManager wm;
    wm.resetSettings();
    
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
}

// --- Standard Handlers ---

void handleGetAccessibleCalendars() {
    if (!checkAuth()) {
        server.send(401, "application/json", "[]");
        return;
    }
    StaticJsonDocument<2048> doc;
    JsonArray arr = doc.to<JsonArray>();
    
    // If Admin, maybe allow seeing everyone? 
    // For now, strictly follow permissions + self + explicit grants.
    // If user is Admin, let's include ALL users for convenience? 
    // The prompt asked for "Admin give access to another people".
    // Let's stick to explicit permissions even for Admin to avoid clutter, 
    // OR Admin can see everything.
    // Let's assume Admin can see all.
    if (currentUsername == "Admin") {
         File file = SPIFFS.open(usersFilePath, FILE_READ);
         StaticJsonDocument<4096> usersDoc;
         deserializeJson(usersDoc, file);
         file.close();
         JsonObject root = usersDoc.as<JsonObject>();
         for (JsonPair kv : root) {
             arr.add(kv.key().c_str());
         }
    } else {
        getAccessibleUsers(currentUsername, arr);
    }

    String res;
    serializeJson(arr, res);
    server.send(200, "application/json", res);
}

void handleGetEvents() {
    if (!checkAuth()) {
        server.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
        return;
    }

    // Determine target user
    String targetUser = currentUsername;
    if (server.hasArg("user") && server.arg("user").length() > 0) {
        targetUser = server.arg("user");
    }

    // Verify Access
    bool allowed = false;
    if (targetUser == currentUsername || currentUsername == "Admin") {
        allowed = true;
    } else {
        StaticJsonDocument<2048> doc;
        JsonArray arr = doc.to<JsonArray>();
        getAccessibleUsers(currentUsername, arr);
        for(JsonVariant v : arr) {
            if (v.as<String>() == targetUser) {
                allowed = true;
                break;
            }
        }
    }

    if (!allowed) {
        server.send(403, "application/json", "{\"error\":\"Forbidden\"}");
        return;
    }

    // Construct filename: /events_USERNAME.json
    String path = "/events_" + targetUser + ".json";
    
    if (SPIFFS.exists(path)) {
        File file = SPIFFS.open(path, FILE_READ);
        server.streamFile(file, "application/json");
        file.close();
    } else {
        server.send(200, "application/json", "{}");
    }
}

void handleSaveEvents() {
    if (!checkAuth()) {
        Serial.println("Save failed: Unauthorized");
        server.send(401, "text/plain", "Unauthorized");
        return;
    }
    
    // User can only save to their OWN calendar
    // (Unless we add 'Edit' permission later)
    String path = "/events_" + currentUsername + ".json";
    
    Serial.print("Saving events for user: "); Serial.println(currentUsername);
    Serial.print("Target file: "); Serial.println(path);

    if (server.hasArg("plain")) {
        String data = server.arg("plain");
        Serial.print("Payload size: "); Serial.println(data.length());
        
        File file = SPIFFS.open(path, FILE_WRITE);
        if (file) {
            file.print(data);
            file.close();
            Serial.println("Save successful!");
            server.send(200, "text/plain", "Saved");
        } else {
            Serial.println("Save failed: File write error");
            server.send(500, "text/plain", "Save failed");
        }
    } else {
        Serial.println("Save failed: No data received");
        server.send(400, "text/plain", "No data");
    }
}


bool createHtmlFile() {
  File file = SPIFFS.open(indexHtmlPath, FILE_WRITE);
  if (!file) return false;
  file.print(indexHtml);
  file.close();
  return true;
}

// Connect to WiFi; falls back to captive portal if no saved credentials
bool connectWiFi() {
  WiFi.mode(WIFI_STA);

  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true); // Enable verbose debug
  wifiManager.setConnectRetries(3); // quick retries using stored creds
  wifiManager.setTimeout(180);      // 3-minute portal timeout

  // CLEAN MENU: Only show "Configure WiFi" and "Restart"
  // Removes: info, param, close, exit, update, erase
  std::vector<const char *> menu = {"wifi", "restart"};
  wifiManager.setMenu(menu);

  // Custom CSS for WiFi Manager to match App Theme
  const char* customHead = R"(
    <style>
      body {
        background-color: #1a202c; 
        color: #e2e8f0; 
        font-family: 'Inter', sans-serif;
        background-image: linear-gradient(135deg, #1a4f72 0%, #2980b9 50%, #1a4f72 100%);
      }
      h1 { color: #63b3ed; text-align: center; font-family: 'Courier New', monospace; }
      div, input, select { padding: 5px; font-size: 1em; margin: 5px 0; box-sizing: border-box; }
      input, select { 
        width: 100%; 
        background: rgba(255, 255, 255, 0.1); 
        color: white; 
        border: 1px solid rgba(255, 255, 255, 0.2); 
        border-radius: 8px;
        padding: 10px;
      }
      button {
        background-color: #3182ce;
        color: white;
        border: none;
        padding: 12px 20px;
        border-radius: 8px;
        cursor: pointer;
        width: 100%;
        font-weight: bold;
        transition: background 0.3s;
      }
      button:hover { background-color: #2c5282; }
      .c { text-align: center; }
      div { 
        background: rgba(255, 255, 255, 0.05); 
        backdrop-filter: blur(10px); 
        border-radius: 12px; 
        padding: 20px; 
        margin-bottom: 20px;
        border: 1px solid rgba(255, 255, 255, 0.1);
      }
      a { color: #63b3ed; text-decoration: none; }
    </style>
  )";
  wifiManager.setCustomHeadElement(customHead);

  // Callback for when it enters AP mode
  wifiManager.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("\n\n------------------------------------------------");
    Serial.println("Could not connect to saved WiFi.");
    Serial.println("Starting configuration Access Point.");
    Serial.println("------------------------------------------------");
    Serial.print("AP SSID: "); Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("AP IP:   "); Serial.println(WiFi.softAPIP());
    Serial.println("------------------------------------------------");
    Serial.println("1. Connect your phone/laptop to the AP SSID above.");
    Serial.println("2. Open browser to http://192.168.4.1");
    Serial.println("3. Configure your home WiFi.");
    Serial.println("------------------------------------------------\n");
  });

  if (!wifiManager.autoConnect("ESP32-Calendar")) {
    Serial.println("WiFi setup timed out or failed; restarting...");
    delay(3000);
    ESP.restart();
    return false;
  }

  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Check for BOOT button (GPIO 0) to reset WiFi settings
  pinMode(0, INPUT_PULLUP);
  // Simple check: if held during startup
  if (digitalRead(0) == LOW) {
      Serial.println("\n!!! BOOT BUTTON HELD !!!");
      Serial.println("Resetting WiFi Manager settings...");
      WiFiManager wm;
      wm.resetSettings();
      Serial.println("Settings cleared. Restarting...");
      delay(2000);
      ESP.restart();
  }
  
  if (!initSPIFFS()) return;

  connectWiFi();

  // Basic Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_GET, handleLoginPage);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/register", HTTP_POST, handleRegister);
  server.on("/logout", HTTP_GET, handleLogout);
  
  // App Routes
  server.on("/events", HTTP_GET, handleGetEvents);
  server.on("/events", HTTP_POST, handleSaveEvents);
  server.on("/accessible_calendars", HTTP_GET, handleGetAccessibleCalendars);

  // Admin Routes
  server.on("/admin", HTTP_GET, handleAdminPage);
  server.on("/admin/users", HTTP_GET, handleAdminGetUsers);
  server.on("/admin/change_password", HTTP_POST, handleAdminChangePass);
  server.on("/admin/delete_user", HTTP_POST, handleAdminDeleteUser);
  server.on("/admin/permissions", HTTP_GET, handleAdminGetPermissions);
  server.on("/admin/permissions", HTTP_POST, handleAdminSetPermissions);
  server.on("/admin/reset_wifi", HTTP_POST, handleAdminResetWiFi);

  // Collect Cookies header
  const char *headerkeys[] = {"Cookie"};
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *);
  server.collectHeaders(headerkeys, headerkeyssize);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(2);
}
