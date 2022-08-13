#include "WebFrontend.h"
#include <EEPROM.h>
#include "Settings.h"
#include "HTML.h"
#include "OTAUpdate.h"
#include "Help.h"
#include "ESP8266WiFiType.h"
#include "ESPTools.h"

String WifiModeToString(WiFiMode_t mode) {
  switch (mode) {
  case WiFiMode_t::WIFI_AP:
    return "Accespoint";
    break;
  case WiFiMode_t::WIFI_AP_STA:
    return "Accespoint + Station";
    break;
  case WiFiMode_t::WIFI_OFF:
    return "Off";
    break;
  case WiFiMode_t::WIFI_STA:
    return "Station";
    break;
  default:
    return "";
    break;
  }
}

const char on_log[] PROGMEM = ""
"<script>\n"
"  function sendCommand() {\n"
"    var cmd = document.getElementById('commandText').value;\n"
"    var request = new XMLHttpRequest();\n"
"    request.open('GET', 'command?cmd=' + encodeURIComponent(cmd), true);\n"
"    request.send();\n"
"  };\n"
"  function clearList(what) {\n"
"    document.getElementById(what + 'Div').innerHTML = '';\n"
"    filter(what);\n"
"  };\n"
"  function filter(what) {\n"
"    var el = document.getElementById(what + 'DivFilter');\n"
"    var text0 = el.value.toLowerCase();\n"
"    var elements = document.getElementsByClassName(what + 'Line');\n"
"    var names = '';\n"
"    var ct = 0;\n"
"    for (var i = 0; i < elements.length; i++) {\n"
"      if (elements[i].innerHTML.toLowerCase().indexOf(text0) == -1) {\n"
"        elements[i].style.display = 'none';\n"
"      }\n"
"      else {\n"
"        elements[i].style.display = 'block';\n"
"        ct++;\n"
"      }\n"
"    }\n"
"    document.getElementById(what + 'RowCount').innerHTML = ct + \" rows\";\n"
"  };\n"
"  function run() {\n"
"    var el = document.getElementById('logDivFilter');\n"
"    el.onkeyup = function (evt) {\n"
"      filter('log');\n"
"    };\n"
"    var el = document.getElementById('dataDivFilter');\n"
"    el.onkeyup = function (evt) {\n"
"      filter('data');\n"
"    };\n"
"    getLogData();\n"
"  };\n"
"  function getLogData() {\n"
"    if (document.getElementById('enabled').checked == true) {\n"
"      var request = new XMLHttpRequest();\n"
"      request.onreadystatechange = function () {\n"
"        if (this.readyState == 4 && this.status == 200 && this.responseText != null && this.responseText != '') {\n"
"          var lines = this.responseText.split('\\n');\n"
"          for (var i = 0; i < lines.length; i++) {\n"
"            var txt = lines[i];\n"
"            if (txt != '') {\n"
"              if (txt == 'SYS: ***CLEARLOG***') {\n"
"                clearList('data');\n"
"                clearList('log');\n"
"              } else {\n"
"                var targetDiv = 'logDiv';\n"
"                var scrollCheckBox = 'scrollLogDiv';\n"
"                var prefix = 'log';\n"
"                if (txt.startsWith('DATA:')) {\n"
"                  prefix = 'data';\n"
"                  targetDiv = 'dataDiv';\n"
"                  scrollCheckBox = 'scrollDataDiv';\n"
"                  txt = txt.substring(5);\n"
"                }\n"
"                if (txt.startsWith('SYS:')) {\n"
"                  txt = txt.substring(4);\n"
"                }\n"
"                txt = new Date().toLocaleTimeString('de-DE') + ': ' + txt;\n"
"                document.getElementById(targetDiv).innerHTML += \"<div class='\" + prefix + \"Line'>\" + txt + '</div>';\n"
"                filter(prefix);\n"
"                if (document.getElementById(scrollCheckBox).checked == true) {\n"
"                  var objDiv = document.getElementById(targetDiv);\n"
"                  objDiv.scrollTop = objDiv.scrollHeight;\n"
"                }\n"
"              }\n"
"            }\n"
"          }\n"
"        }\n"
"      };\n"
"      request.open('GET', 'getLogData?nc=' + Math.random(), true);\n"
"      request.send();\n"
"    }\n"
"    setTimeout('getLogData()', 500);\n"
"  };\n"
"</script>\n"
"<body onload='run()'>\n"
"  Command: <input id='commandText' size='100' onkeydown=\"if (event.keyCode == 13) sendCommand()\">\n"
"  <button type='button' onclick=\"sendCommand();\">Send</button>\n"
"  &nbsp;&nbsp;&nbsp;\n"
"  <input type='checkbox' id='enabled' value='true' checked>Enable logging\n"
"  <br><br>\n"
"  <i>LGW to FHEM:</i>\n"
"  <input type='checkbox' id='scrollDataDiv' value='true' checked> Scroll\n"
"  <button type='button' onclick=\"clearList('data');\">Clear</button>\n"
"  Filter:\n"
"  <input id='dataDivFilter'>\n"
"  <span id='dataRowCount'></span>\n"
"  <div id='dataDiv' style='height: 250px; border:1px solid black; overflow:scroll;'></div>\n"
"  <br><i>Debug log:</i>\n"
"  <input type='checkbox' id='scrollLogDiv' value='true' checked> Scroll\n"
"  <button type='button' onclick=\"clearList('log');\">Clear</button>\n"
"  Filter:\n"
"  <input text='text' id='logDivFilter'>\n"
"  <span id='logRowCount'></span>\n"
"  <div id='logDiv' style='height: 250px; border:1px solid black; overflow:scroll;'></div>\n"
"</body>\n"
;

WebFrontend::WebFrontend(int port) : m_webserver(port) {
  m_port = port;
  m_password = "";
  m_commandCallback = nullptr;
  m_hardwareCallback = nullptr;
}

ESP8266WebServer *WebFrontend::WebServer() {
  return &m_webserver;
}

void WebFrontend::SetCommandCallback(CommandCallbackType callback) {
  m_commandCallback = callback;
}

void WebFrontend::SetHardwareCallback(HardwareCallbackType callback) {
  m_hardwareCallback = callback;
}

void WebFrontend::SetPassword(String password) {
  m_password = password;
}

bool WebFrontend::IsAuthentified() {
  bool result = false;
  if (m_password.length() > 0) {
    if (m_webserver.hasHeader("Cookie")) {
      String cookie = m_webserver.header("Cookie");
      if (cookie.indexOf("ESPSESSIONID=1") != -1) {
        result = true;
      }
    }
    if (!result) {
      String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
      m_webserver.sendContent(header);
    }
  }
  else {
    result = true;
  }

  return result;
}

String GetOption(String option, String defaultValue) {
  String result = "";

  result += F("<option value='");
  result += option;
  if (defaultValue == option) {
    result += F("' selected>");
  }
  else {
    result += F("'>");
  }
  result += option;
  result += F("</option>");

  return result;
}

void WebFrontend::Begin(StateManager *stateManager, Logger *logger) {
  m_stateManager = stateManager;
  m_logger = logger;
  
  const char *headerKeys[] = { "User-Agent", "Cookie" };
  m_webserver.collectHeaders(headerKeys, sizeof(headerKeys) / sizeof(char*));

  m_webserver.on("/", [this]() {
    if (IsAuthentified()) {
      String result;
      result += GetTop();
      result += GetNavigation();
      result += F("<br>");
      result += m_stateManager->GetHTML();
      result += GetBottom();
      m_webserver.send(200, "text/html", result);
    }
  });

  m_webserver.on("/reset", [this]() {
    if (IsAuthentified()) {
      m_webserver.send(200, "text/html", GetRedirectToRoot());
      delay(1000);
      ESP.restart();
    }
  });

  m_webserver.on("/command", [this]() {
    if (IsAuthentified()) {
      if (m_commandCallback != NULL) {
        String command = m_webserver.arg("cmd");
        m_logger->println("Command from frontend: '" + command + "'");
        m_commandCallback(command);
        m_webserver.send(200, "text/html", "OK");
      }
    }
  });

  m_webserver.on("/state", [this]() {
    String result;
    result += m_stateManager->GetXML();
    m_webserver.send(200, "text/xml", result);
  });

  m_webserver.on("/help", [this]() {
    if (IsAuthentified()) {
      String result;
      result += GetTop();
      result += GetNavigation();
      result += FPSTR(help);
      result += GetBottom();
      m_webserver.send(200, "text/html", result);
    }
  });

  m_webserver.on("/hardware", [this]() {
    if (IsAuthentified()) {
      uint32_t freeHeap = ESP.getFreeHeap();
      String result;

      m_webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
      m_webserver.send(200);

      result += GetTop();
      result += GetNavigation();
      result += "<br><table>";
      m_webserver.sendContent(result);
      result = "";

      result += BuildHardwareRow("ESP8266&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;", "present :-)&nbsp;&nbsp;&nbsp;", "Core:&nbsp;" + String(ESP.getCoreVersion()) + "&nbsp;&nbsp;SDK:&nbsp;" + String(ESP.getSdkVersion()) + "&nbsp;&nbsp;free heap:&nbsp;" + String(freeHeap) + "&nbsp;&nbsp;Reset:&nbsp;" + ESP.getResetReason() + "&nbsp;&nbsp;->&nbsp;" + ESP.getResetInfo());
      m_webserver.sendContent(result);
      result = "";

      result += BuildHardwareRow("WiFi", String(WiFi.RSSI()) + " dBm", "Mode: " + WifiModeToString(WiFi.getMode()) + "&nbsp;&nbsp;&nbsp;Time to connect: " + String(m_stateManager->GetWiFiConnectTime(), 1) + " s");
      m_webserver.sendContent(result);
      result = "";

      if (m_hardwareCallback != nullptr) {
        String rawData = m_hardwareCallback();
        result += "<tr><td>";
        rawData.replace("\t", "</td><td>");
        rawData.replace("\n", "</td></tr><tr><td>");
        rawData.replace(" ", "&nbsp;");
        result += rawData;
        result += "</td></tr></table>";
      }
      m_webserver.sendContent(result);

      m_webserver.sendContent(GetBottom());
      m_webserver.sendContent("");
    }
  });

  m_webserver.on("/ota", [this]() {
    if (IsAuthentified()) {
      String result;
      result += GetTop();
      result += GetNavigation();
      Settings settings;
      settings.Read(m_logger);
      result += F("<br><form method='get' action='ota_start'>");
      result += F("Server:&nbsp");
      result += settings.Get("otaServer", "");
      result += F("<br>Port:&nbsp");
      result += settings.Get("otaPort", "");
      result += F("<br>url:&nbsp");
      result += settings.Get("otaURL", "");
      result += F("<br><br> <input type='submit' Value='Update and restart' > </form>");
      result += GetBottom();
      m_webserver.send(200, "text/html", result);
    }
  });

  m_webserver.on("/ota_start", [this]() {
    if (IsAuthentified()) {
      m_webserver.send(200, "text/html", OTAUpdate::Start(m_logger));
    }
  });

  m_webserver.on("/save", [this]() {
    if (IsAuthentified()) {
      Settings settings;

      bool gotUseWiFi = false;
      for (byte i = 0; i < m_webserver.args(); i++) {
        settings.Add(m_webserver.argName(i), m_webserver.arg(i));
        if (m_webserver.argName(i) == "UseWiFi") {
          gotUseWiFi = true;
        }
      }
      if (!gotUseWiFi) {
        settings.Add("UseWiFi", "false");
      }

      bool saveIt = true;
      if (m_webserver.hasArg("frontPass") && m_webserver.hasArg("frontPass2")) {
        String fp1 = m_webserver.arg("frontPass");
        String fp2 = m_webserver.arg("frontPass2");
        if (!fp1.equals(fp2)) {
          String content = GetTop();
          content += F("<div align=center>");
          content += F("<br><br><h2><font color='red'>");
          content += F("Passwords do nat match</font></h2>");
          content += F("</div>");
          content += GetBottom();
          m_webserver.send(200, "text/html", content);
          saveIt = false;
        }
      }

      if (m_webserver.hasArg("HostName")) {
        String hostname = m_webserver.arg("HostName");
        for (byte i = 0; i < hostname.length(); i++) {
          char ch = (char)hostname[i];
          if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '-' || ch == '_')) {
            saveIt = false;
            String content = GetTop();
            content += F("<div align=center>");
            content += F("<br><br><h2><font color='red'>");
            content += F("Allowed characters for hostname: 0...9, a...z, A...Z, - and _</font></h2>");
            content += F("</div>");
            content += GetBottom();
            m_webserver.send(200, "text/html", content);
            break;
          }

        }
      }
      
      if (saveIt) {
        String info = settings.Write();
        m_webserver.send(200, "text/html", GetRedirectToRoot("Settings saved<br>" + info));
        delay(1000);
        ESP.restart();
      }

    }
  });

  m_webserver.on("/setup", [this]() {
    if (IsAuthentified()) {
      String result;
 
      Settings settings;
      settings.Read(m_logger);

      m_webserver.setContentLength(CONTENT_LENGTH_UNKNOWN);
      m_webserver.send(200);

      m_webserver.sendContent(GetTop() + GetNavigation());

      String data;
      data += F("<form method='get' action='save'><table>");
      data += F("<tr><td></td><td><br>Third parameter is the timeout. After timeout seconds, it will try to connect to SSID2 (if defined)</td></tr>");

      // ctSSID ctPASS
      data += F("<tr><td><label>SSID / Password: </label></td><td><input name='ctSSID' size='44' maxlength='32' Value='");
      data += settings.Get("ctSSID", "");
      data += F("'>");
      data += F("&nbsp;&nbsp;<input type='password' name='ctPASS' size='54' maxlength='63' Value='");
      data += settings.Get("ctPASS", "");
      data += F("'>&nbsp;&nbsp;<input name='Timeout1' size='6' maxlength='4' Value='");
      data += settings.Get("Timeout1", "15");
      data += F("'> </td></tr>");
      
      // ctSSID2 ctPASS2
      data += F("<tr><td><label>SSID2 / Password2: </label> </td> <td> <input name='ctSSID2' size='44' maxlength='32' Value='");
      data += settings.Get("ctSSID2", "");
      data += F("'>");
      data += F("&nbsp;&nbsp;<input type='password' name='ctPASS2' size='54' maxlength='63' Value='");
      data += settings.Get("ctPASS2", "");
      data += F("'>&nbsp;&nbsp;<input name='Timeout2' size='6' maxlength='4' Value='");
      data += settings.Get("Timeout2", "15");
      data += F("'> </td></tr>");
      
      m_webserver.sendContent(data);
      data = "";

      // Frontend Password
      data += F("<tr><td><label>Frontend password: </label> </td> <td>");
      data += F("<input name='frontPass' type='password' size='30' maxlength='60' Value='");
      data += settings.Get("frontPass", "");
      data += F("'> Retype: ");
      data += F("<input name='frontPass2' type='password' size='30' maxlength='60' Value='");
      data += settings.Get("frontPass2", "");
      data += F("'> (if empty, no login is required)</td></tr>");

      data += F("<tr><td></td><td><br>MQTT server settings</td></tr>");
      // serverIpMqtt, serverPortMqtt 
      data += F("<tr> <td> <label>IP-Address: </label></td><td><input name='serverIpMqtt' size='27' maxlength='15' Value='");
      data += settings.Get("serverIpMqtt", "");
      data += F("'><label>&nbsp;&nbsp;Port: </label><input name='serverPortMqtt' size='27' maxlength='15' Value='");
      data += settings.Get("serverPortMqtt", "1883");
      data += F("'></td></tr>");
      // serverIpMqtt, serverPortMqtt 
      data += F("<tr> <td> <label>Username: </label></td><td><input name='mqttUser' size='44' maxlength='32' Value='");
      data += settings.Get("mqttUser", "");
      data += F("'><label>&nbsp;&nbsp;Password: </label><input type='password' name='mqttPass' size='54' maxlength='63' Value='");
      data += settings.Get("mqttPass", "");
      data += F("'></td></tr>");

      // Other MQTT settings
      data += F("<tr><td><label>MQTT settings:</label></td><td><label>publish Interval: </label><input name='pubInt' size='5' maxlength='5' Value='");
      data += settings.Get("pubInt", "20");

      data += F("'><label>&nbsp;&nbsp;Topic:  </label><input name='topic' size='27' maxlength='63' Value='");
      data += settings.Get("topic", "10");

      data += F("'><label>&nbsp;&nbsp;Ext1: </label><input name='ext1' size='5' maxlength='4' Value='");
      data += settings.Get("ext1", "0");
      data += F("'><label>&nbsp;&nbsp;Ext2: </label><input name='ext2' size='5' maxlength='5' Value='");
      data += settings.Get("ext2", "0");
      data += F("'><label>&nbsp;&nbsp;Ext3: </label><input name='ext3' size='5' maxlength='5' Value='");
      data += settings.Get("ext3", "0");
      data += F("'></td></tr>");

      data += F("<tr><td></td><td><br>DHCP will be used, in case of one of the fields IP, mask or gateway remains empty</td></tr>");

      // staticIP, staticMask, staticGW, 
      data += F("<tr> <td> <label>IP-Address: </label></td><td><input name='staticIP' size='27' maxlength='15' Value='");
      data += settings.Get("staticIP", "");
      data += F("'><label>&nbsp;&nbsp;Netmask: </label><input name='staticMask' size='27' maxlength='15' Value='");
      data += settings.Get("staticMask", "");
      data += F("'><label>&nbsp;&nbsp;Gateway: </label><input name='staticGW' size='27' maxlength='15' Value='");
      data += settings.Get("staticGW", "");
      data += F("'></td></tr>");

      // HostName, startup-delay
      data += F("<tr><td><label>Hostname: </label></td><td><input name='HostName' size='27' maxlength='63' Value='");
      data += settings.Get("HostName", "LaCrosseGateway");
      data += F("'><label>&nbsp;&nbsp;Startup-delay (s): </label><input name='StartupDelay' size='5' maxlength='4' Value='");
      data += settings.Get("StartupDelay", "0");
      data += F("'> </td></tr>");

      m_webserver.sendContent(data);
      data = "";

      // Internal sensors
      data += F("<tr><td><label>Internal sensors:</label></td><td><label>ID: </label><input name='ISID' size='5' maxlength='4' Value='");
      data += settings.Get("ISID", "0");

      data += F("'><label>&nbsp;&nbsp;Interval: </label><input name='ISIV' size='5' maxlength='5' Value='");
      data += settings.Get("ISIV", "10");

      data += F("'><label>&nbsp;&nbsp;Altitude: </label><input name='Altitude' size='5' maxlength='4' Value='");
      data += settings.Get("Altitude", "0");
      data += F("'><label>&nbsp;&nbsp;Temperature-correction: </label><input name='CorrT' size='5' maxlength='5' Value='");
      data += settings.Get("CorrT", "0");
      data += F("'><label>&nbsp;&nbsp;Humidity-correction: </label><input name='CorrH' size='5' maxlength='5' Value='");
      data += settings.Get("CorrH", "0");
      data += F("'></td></tr>");
     
      // Data ports
      data += F("<tr> <td> <label>Data ports: </label></td><td>");
      data += F("<input name='DataPort1' maxlength='5' size='10' Value='");
      data += settings.Get("DataPort1", "81");
      data += F("'>&nbsp;");
      data += F("<input name='DataPort2' maxlength='5' size='10' Value='");
      data += settings.Get("DataPort2", "");
      data += F("'>&nbsp;");
      data += F("<input name='DataPort3' maxlength='5' size='10' Value='");
      data += settings.Get("DataPort3", "");
      data += F("'>&nbsp;");
      data += F("</td></tr>");

      // Serial Bridge 1
      data += F("<tr><td><label>Serial bridge 1:</label></td><td>port:&nbsp;");
      data += F("<input name='SerialBridgePort' maxlength='5' size='10' Value='");
      data += settings.Get("SerialBridgePort", "");
      data += F("'>&nbsp;<label>baud: </label><input name='SerialBridgeBaud' maxlength='6' size='10' Value='");
      data += settings.Get("SerialBridgeBaud", "57600");
      data += F("'>&nbsp;</td></tr>");
      
      // Serial Bridge 2
      data += F("<tr><td><label>Serial bridge 2:</label></td> <td>port:&nbsp");
      data += F("<input name='SerialBridge2Port' maxlength='5' size='10' Value='");
      data += settings.Get("SerialBridge2Port", "");
      data += F("'>&nbsp;<label>baud: </label><input name='SerialBridge2Baud' maxlength='6' size='10' Value='");
      data += settings.Get("SerialBridge2Baud", "57600");
      data += F("'>&nbsp;</td></tr>");
      
      m_webserver.sendContent(data);
      data = "";

      // Soft serial bridge
      data += F("<tr><td><label>Soft serial bridge: </label> </td> <td>");
      data += F("<label>port: </label><input name='SSBridgePort' maxlength='5' size='10' Value='");
      data += settings.Get("SSBridgePort", "");
      data += F("'>&nbsp;<label>baud: </label><input name='SSBridgeBaud' maxlength='6' size='10' Value='");
      data += settings.Get("SSBridgeBaud", "9600");
      data += F("'>&nbsp;");
      data += F("<input name='IsNextion' type='checkbox' value='true' "); data += settings.Get("IsNextion", "") == "true" ? "checked" : ""; data += F(">Nextion display&nbsp;&nbsp;&nbsp;");
      data += F("<input name='AddUnits' type='checkbox' value='true' "); data += settings.Get("AddUnits", "") == "true" ? "checked" : ""; data += F(">Add units");
      data += F("</td></tr>");
      
      data += F("<tr><td> <label>RFM95: </label></td><td>");
      data += F("<label>SF:</label>&nbsp;");
      data += F("<select name='SF95' style='width:60px'>");
      String sfValue = settings.Get("SF95", "SF7");
      data += GetOption("SF6", sfValue);
      data += GetOption("SF7", sfValue);
      data += GetOption("SF8", sfValue);
      data += GetOption("SF9", sfValue);
      data += GetOption("SF10", sfValue);
      data += GetOption("SF11", sfValue);
      data += GetOption("SF12", sfValue);
      data += F("</select>&nbsp;&nbsp;");
      data += F("<label>BW:</label>&nbsp;");
      data += F("<select name='BW95' style='width:60px'>");
      String bwValue = settings.Get("BW95", "125");
      data += GetOption("7.8", bwValue);
      data += GetOption("10.4", bwValue);
      data += GetOption("15.6", bwValue);
      data += GetOption("20.8", bwValue);
      data += GetOption("31.25", bwValue);
      data += GetOption("41.7", bwValue);
      data += GetOption("62.6", bwValue);
      data += GetOption("125", bwValue);
      data += GetOption("250", bwValue);
      data += GetOption("500", bwValue);
      data += F("</select>&nbsp;&nbsp;");
      data += F("</td></tr>");

      // Flags
      data += F("<tr><td><label>Flags: </label></td><td>");
      data += F("<input name='UseWiFi' type='checkbox' value='true' "); data += settings.Get("UseWiFi", "true") == "true" ? "checked" : ""; data += F(">Use WiFi&nbsp;&nbsp;&nbsp;");
      data += F("<input name='UseMDNS' type='checkbox' value='true' "); data += settings.Get("UseMDNS", "") == "true" ? "checked" : ""; data += F("> Use MDNS&nbsp;&nbsp;&nbsp;");
      data += F("<input name='SendAnalog' type='checkbox' value='true' "); data += settings.Get("SendAnalog", "") == "true" ? "checked" : ""; data += F("> Send analog values&nbsp;&nbsp;&nbsp;");
      data += F("&nbsp;<label>U analog 1023: </label><input name='UAnalog1023' maxlength='5' size='7' Value='"); data += settings.Get("UAnalog1023", "1000"); data += F("'>&nbsp;<label>mV</label>");
      data += F("<input name='PRD' type='checkbox' value='true' "); data += settings.Get("PRD", "false") == "true" ? "checked" : ""; data += F(">Pressure with decimals&nbsp;&nbsp;&nbsp;");
      data += F("<br></td></tr>");

      m_webserver.sendContent(data);
      data = "";

      // MCP23008
      data += F("<tr><td> <label>MCP23008: </label></td><td>");
      for (byte nbr = 0; nbr < 8; nbr++) {
        data += F("<label>IO ");
        data += String(nbr);
        data += F(":</label>&nbsp;");
        data += F("<select name='IO");
        data += String(nbr);
        data += F("' style='width:130px'>");
        String value = settings.Get("IO" + String(nbr), "Input");
        data += GetOption("Input", value);
        data += GetOption("Output", value);
        data += GetOption("OLED Off", value);
        data += GetOption("OLED On", value);
        data += GetOption("OLED mode=s", value);
        data += GetOption("OLED mode=t", value);
        data += GetOption("OLED mode=h", value);
        data += GetOption("OLED mode=th", value);
        data += GetOption("OLED mode=thp", value);
        data += GetOption("OLED mode=thps", value);
        data += F("</select>&nbsp;&nbsp;");

        if (nbr == 3) {
          data += F("<br>");
        }
        m_webserver.sendContent(data);
        data = "";
      }
      data += F("</td></tr>");

      // OLED
      data += F("<tr><td></td><td><br>Possible values: 'on', 'off' or the number of seconds until 'off' and in the second parameter a mode like th, thp, ...</td></tr>");
      data += F("<tr> <td> <label>OLED start: </label> </td> <td>on/off: <input name='oledStart' size='10' maxlength='6' Value='");
      data += settings.Get("oledStart", "on");
      data += F("'>&nbsp;mode: ");
      data += F("<input name='oledMode' size='12' maxlength='16' Value='");
      data += settings.Get("oledMode", "");
      data += F("'>&nbsp;&nbsp;");
      data += F("<input name='oled13' type='checkbox' value='true' ");
      data += settings.Get("oled13", "false") == "true" ? "checked" : ""; 
      data += F(">1.3\"");
      data += F("</td></tr>");

      // KVInterval and KVIdentity
      data += F("<tr><td></td><td><br>Use 'off' to disable KV-transmission</td></tr>");
      data += F("<tr> <td> <label>KV-Interval: </label> </td> <td> <input name='KVInterval' size='50' maxlength='3' Value='");
      data += settings.Get("KVInterval", "10");
      data += F("'> <label>KV-Identity: </label> <input name='KVIdentity' size='50' maxlength='20' Value='");
      data += settings.Get("KVIdentity", String(ESP.getChipId()));
      data += F("'></td></tr>");

      m_webserver.sendContent(data);
      data = "";

      // OTA-Server
      data += F("<tr><td><label>OTA-Server: </label></td><td><input name='otaServer' size='120' maxlength='40' Value='");
      data += settings.Get("otaServer", "");
      data += F("'></td></tr>");

      // OTA-Port
      data += F("<tr><td><label>OTA-Port: </label></td><td><input name='otaPort' size='120' maxlength='5' Value='");
      data += settings.Get("otaPort", "");
      data += F("'></td></tr>");

      // OTA-url
      data += F("<tr><td><label>OTA-url: </label></td><td><input name='otaURL' size='120' maxlength='80' Value='");
      data += settings.Get("otaURL", "");
      data += F("'></td></tr>");

      m_webserver.sendContent(data);
      data = "";

      // PCA301
      data += F("<tr><td><label>PCA301: </label></td><td><input name='PCA301Plugs' size='120' maxlength='160' Value='");
      data += settings.Get("PCA301Plugs", "");
      data += F("'></td></tr>");

      // Flags
      data += F("<tr><td></td><td><br>Only for development</td></tr>");
      data += F("<tr><td><label>Flags: </label> </td> <td> <input name='Flags' size='90' maxlength='80' Value='");
      data += settings.Get("Flags", "");
      data += F("'></td></tr>");

      data += F("</table><br><input type='submit' Value='Save and restart' ></form>");

      m_webserver.sendContent(data);
      data = "";

      m_webserver.sendContent(GetBottom());

      m_webserver.sendContent("");
    }
  });
  
  m_webserver.on("/getLogData", [this]() {
    String data = "";
    if (m_logger->IsEnabled()) {
      while (m_logger->Available()) {
        data += m_logger->Pop() + "\n";
      }
    }
    else {
      data += F("SYS: ***CLEARLOG***\n");
      data += F("DATA:Logger is disabled\n");
      data += F("SYS:Logger is disabled\n");
    }

    m_webserver.send(200, "text/html", data);
  });

  m_webserver.on("/log", [this]() {
    if (IsAuthentified()) {
      String result;
      result += GetTop();
      result += GetNavigation();
      result += F("<br>");
      result += FPSTR(on_log);

      result += GetBottom();

      m_webserver.send(200, "text/html", result);
    }
  });

  m_webserver.on("/login", [this]() {
    String msg;
    if (m_webserver.hasArg("DISCONNECT")) {
      m_webserver.sendContent(F("HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=0\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n"));
      return;
    }
    if (m_webserver.hasArg("PASSWORD")) {
      if (m_webserver.arg("PASSWORD") == m_password) {
        m_webserver.sendContent(F("HTTP/1.1 301 OK\r\nSet-Cookie: ESPSESSIONID=1\r\nLocation: /\r\nCache-Control: no-cache\r\n\r\n"));
        return;
      }
      msg = "Login failed";
    }
    String content = F("<html><body><form action='/login' method='POST'>");
    content += F("<DIV ALIGN=CENTER>");
    content += F("<br><br><h2>LaCrosseGateway V");
    content += m_stateManager->GetVersion();
    content += F("</h2>");
    content += F("Password: <input type='password' name='PASSWORD'>&nbsp;&nbsp;");
    content += F("<input type='submit' name='SUBMIT' value='Login'></form>");
    content += F("<br><br><h2> <font color='red'>");
    content += msg;
    content += F("</font></h2>");
    content += F("</div>");
    content += F("</body></html>");
    m_webserver.send(200, "text/html", content);
  });

  m_webserver.onNotFound([this](){
    m_webserver.send(404, "text/plain", "Not Found");
  });

  m_webserver.begin();
}

String WebFrontend::GetNavigation() {
  String result = "";
  result += F("<a href='/'>Home</a>&nbsp;&nbsp;");
  result += F("<a href='setup'>Setup</a>&nbsp;&nbsp;");
  result += F("<a href='hardware'>Hardware</a>&nbsp;&nbsp;");
  result += F("<a href='ota'>OTA-Update</a>&nbsp;&nbsp;");
  result += F("<a href='log'>Log</a>&nbsp;&nbsp;");
  result += F("<a href='help'>Help</a>&nbsp;&nbsp;");
  if (m_password.length() > 0) {
    result += F("<a href='login?DISCONNECT=YES'>Logout</a>&nbsp;&nbsp;");
  }
  result += F("<a href='reset'>Reboot</a>");
  result += F("<br>");
  
  return result;
}

String WebFrontend::GetDisplayName() {
  String result;
  result += m_stateManager->GetHostname();
  result += " (";
  result += WiFi.localIP().toString();
  result += ")";
  return result;
}

String WebFrontend::GetTop() {
  String result;
  result += F("<!DOCTYPE HTML><html>");
  result += F("<meta charset='utf-8'/>");
  result += "<head><title>";
  result += GetDisplayName();
  result += "</title></head>";
  result += F("<p>LaCrosseGateway V");
  result += m_stateManager->GetVersion();
  result += "&nbsp;&nbsp;&nbsp;";
  result += GetDisplayName();
  result += F("</p>");
  return result;
}

String WebFrontend::GetBottom() {
  String result;
  result += F("</html>");
  return result;
}

String WebFrontend::GetRedirectToRoot(String message) {
  String result;
  result += F("<html><head><meta http-equiv='refresh' content='5; URL=/'></head><body>");
  result += message;
  result += F("<br><br>Reboot, please wait a moment ...</body></html>");
  return result;
}

String WebFrontend::BuildHardwareRow(String text1, String text2, String text3) {
  return "<tr><td>" + text1 + "</td><td>" + text2 + "</td><td>" + text3 + "</td></tr>";
}

void WebFrontend::Handle() {
  m_webserver.handleClient();
}

