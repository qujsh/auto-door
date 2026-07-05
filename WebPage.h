#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>Auto Door V3</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:#0f0f1a;color:#e0e0e0;max-width:440px;margin:0 auto;padding:12px}
h1{text-align:center;font-size:18px;margin:8px 0 2px;letter-spacing:1px}
.sub{text-align:center;font-size:11px;color:#555;margin-bottom:14px}
.card{background:#1a1a2e;border-radius:10px;padding:14px;margin-bottom:10px}
.row{display:flex;justify-content:space-between;align-items:center;padding:5px 0;border-bottom:1px solid #1f1f35}
.row:last-child{border-bottom:none}
.lbl{color:#777;font-size:13px}
.val{font-size:16px;font-weight:600}
.g{color:#4ade80}.r{color:#f87171}.y{color:#facc15}
.btn{background:#1e3a5f;color:#e0e0e0;border:none;border-radius:8px;padding:10px 14px;font-size:15px;cursor:pointer;transition:background .15s}
.btn:active{background:#533483}
.btn.primary{background:#533483;font-weight:600}
.btn-row{display:flex;gap:8px;margin-top:10px}
.btn-row .btn{flex:1}
.slider-wrap{margin:4px 0}
input[type=range]{width:100%;height:6px;-webkit-appearance:none;appearance:none;background:#1e3a5f;border-radius:3px;outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;background:#533483;border-radius:50%;cursor:pointer}
.angle-val{text-align:center;font-size:28px;font-weight:700;margin:4px 0 2px}
.mode-row{display:flex;gap:20px;align-items:center;padding:4px 0}
.mode-row label{cursor:pointer;font-size:15px}
.mode-row input[type=radio]{accent-color:#533483;width:16px;height:16px;cursor:pointer}
#log{background:#0a0a14;border-radius:8px;padding:8px;font-size:11px;max-height:120px;overflow-y:auto;font-family:monospace;margin-top:10px}
.log-line{padding:2px 0;border-bottom:1px solid #141428;color:#888}
</style>
</head>
<body>
<h1>Auto Door V3</h1>
<div class="sub">ESP32 Smart Door Control</div>

<div class="card">
<div class="row"><span class="lbl">WiFi</span><span id="wifi" class="val g">-</span></div>
<div class="row"><span class="lbl">IP</span><span id="ip" class="val">-</span></div>
<div class="row"><span class="lbl">RSSI</span><span id="rssi" class="val">-</span></div>
</div>

<div class="card">
<div class="row"><span class="lbl">Distance</span><span id="dist" class="val">-</span></div>
<div class="row"><span class="lbl">Baseline</span><span id="base" class="val">-</span></div>
<div class="row"><span class="lbl">Diff</span><span id="diff" class="val">-</span></div>
<div class="row"><span class="lbl">Detect</span><span id="detect" class="val r">-</span></div>
<div class="row"><span class="lbl">Present</span><span id="present" class="val r">-</span></div>
<div class="row"><span class="lbl">Door State</span><span id="door" class="val">-</span></div>
<div class="row"><span class="lbl">Servo</span><span id="servo" class="val">-</span></div>
</div>

<div class="card">
<div class="lbl" style="margin-bottom:6px">Mode</div>
<div class="mode-row">
<label><input type="radio" name="mode" value="auto" onchange="setMode('auto')"> AUTO</label>
<label><input type="radio" name="mode" value="manual" onchange="setMode('manual')"> MANUAL</label>
</div>
</div>

<div class="card">
<div class="lbl">Servo Angle</div>
<div class="angle-val"><span id="angleDisp">0</span>&#176;</div>
<div class="slider-wrap"><input type="range" id="slider" min="0" max="180" value="0" oninput="onSlider()" onchange="onSlider()"></div>
<div class="btn-row">
<button class="btn" onclick="setServo(0)">CLOSE</button>
<button class="btn primary" onclick="setServo(90)">OPEN</button>
<button class="btn" onclick="calibrate()">CALIBRATE</button>
</div>
</div>

<div id="log"></div>

<script>
var ip='',autoAngle=0;
function onSlider(){var v=document.getElementById('slider').value;document.getElementById('angleDisp').textContent=v;setServo(v)}
function setServo(a){document.getElementById('slider').value=a;document.getElementById('angleDisp').textContent=a;fetch('/api/servo?angle='+a)}
function setMode(m){fetch('/api/mode',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"mode":"'+m+'"}'})}
function calibrate(){fetch('/api/calibrate',{method:'POST'}).then(function(r){return r.json()}).then(function(d){if(d.baseline)document.getElementById('base').textContent=d.baseline.toFixed(2)+' cm'})}
function addLog(m){var l=document.getElementById('log'),t=new Date().toLocaleTimeString();l.innerHTML+='<div class="log-line">'+t+' '+m+'</div>';l.scrollTop=l.scrollHeight;while(l.children.length>50)l.removeChild(l.firstChild)}
function update(){
  fetch('/api/status').then(function(r){return r.json()}).then(function(d){
    var w=document.getElementById('wifi');w.textContent=d.wifi?'Connected':'Disconnected';w.className='val '+(d.wifi?'g':'r');
    document.getElementById('ip').textContent=d.ip||'-';
    document.getElementById('rssi').textContent=d.rssi!=null?d.rssi+' dBm':'-';
    if(d.distance>=0)document.getElementById('dist').textContent=d.distance.toFixed(2)+' cm';
    document.getElementById('base').textContent=d.baseline.toFixed(2)+' cm';
    document.getElementById('diff').textContent=d.diff.toFixed(2)+' cm';
    var dt=document.getElementById('detect');dt.textContent=d.detect?'YES':'NO';dt.className='val '+(d.detect?'g':'r');
    var pr=document.getElementById('present');pr.textContent=d.present?'YES':'NO';pr.className='val '+(d.present?'g':'r');
    var dr=document.getElementById('door');dr.textContent=d.door;
    if(d.door==='OPEN')dr.className='val g';
    else if(d.door==='WAIT_CLOSE')dr.className='val y';
    else dr.className='val';
    document.getElementById('servo').textContent=d.servo+'°';
    if(d.mode==='AUTO'){document.querySelector('input[value=auto]').checked=true}
    else{document.querySelector('input[value=manual]').checked=true}
    if(d.mode!=='MANUAL'){autoAngle=d.servo;document.getElementById('slider').value=autoAngle;document.getElementById('angleDisp').textContent=autoAngle}
  }).catch(function(e){document.getElementById('wifi').textContent='Offline';document.getElementById('wifi').className='val r'})
}
setInterval(update,500);
update();
</script>
</body>
</html>
)=====";

const char WIFI_SETUP_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>AutoDoor Setup</title>
<style>
*,*::before,*::after{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:#0f0f1a;color:#e0e0e0;max-width:400px;margin:0 auto;padding:16px}
h1{text-align:center;font-size:20px;margin:12px 0 4px}
.sub{text-align:center;font-size:12px;color:#555;margin-bottom:16px}
.card{background:#1a1a2e;border-radius:10px;padding:14px;margin-bottom:10px}
.network{display:flex;align-items:center;padding:10px 8px;border-bottom:1px solid #1f1f35;cursor:pointer;border-radius:6px}
.network:hover{background:#1e3a5f}
.network.selected{background:#533483}
.network-name{flex:1;font-size:15px;font-weight:500}
.network-rssi{font-size:12px;color:#888;margin-left:8px}
.network-lock{color:#888;margin-left:4px;font-size:13px}
#network-list{max-height:260px;overflow-y:auto}
#password-section{display:none;margin-top:10px}
#password-section input{width:100%;background:#0f0f1a;border:1px solid #333;border-radius:6px;padding:10px;color:#e0e0e0;font-size:15px}
.btn{display:block;width:100%;background:#533483;color:#e0e0e0;border:none;border-radius:8px;padding:12px;font-size:16px;font-weight:600;cursor:pointer;margin-top:12px}
.btn:active{opacity:.8}
.btn:disabled{opacity:.4}
.btn.secondary{background:#1e3a5f;margin-top:6px}
#status{text-align:center;font-size:13px;margin-top:8px;min-height:20px}
.fade{opacity:.5}
</style>
</head>
<body>
<h1>AutoDoor V3</h1>
<div class="sub">WiFi Setup</div>

<div class="card">
<div style="font-size:13px;color:#888;margin-bottom:10px">Select your WiFi network:</div>
<div id="network-list">
<div style="text-align:center;padding:20px;color:#555">Scanning...</div>
</div>
</div>

<div id="password-section" class="card">
<div style="font-size:13px;color:#888;margin-bottom:6px">Password for <b id="selected-name"></b></div>
<input type="password" id="password" placeholder="Enter WiFi password">
</div>

<button class="btn" id="save-btn" disabled onclick="save()">Connect</button>

<div id="status"></div>

<script>
var selectedSSID='',selectedSecure=false;
function scan(){
  fetch('/api/wifi/scan').then(function(r){return r.json()}).then(function(nets){
    var h='';
    nets.sort(function(a,b){return b.rssi-a.rssi});
    nets.forEach(function(n){
      var dbm=n.rssi>=-50?'green':n.rssi>=-70?'#facc15':'#f87171';
      h+='<div class="network" onclick="select(\''+n.ssid+'\','+n.secure+')" id="net-'+n.ssid.replace(/'/g,'')+'">';
      h+='<span class="network-name">'+n.ssid+'</span>';
      h+='<span class="network-rssi" style="color:'+dbm+'">'+n.rssi+' dBm</span>';
      h+=n.secure?'<span class="network-lock">&#x1f512;</span>':'<span class="network-lock">open</span>';
      h+='</div>';
    });
    if(nets.length===0)h='<div style="text-align:center;padding:20px;color:#555">No networks found</div>';
    document.getElementById('network-list').innerHTML=h;
  }).catch(function(){document.getElementById('network-list').innerHTML='<div style="text-align:center;padding:20px;color:#f87171">Scan failed</div>'});
}
function select(ssid,secure){
  selectedSSID=ssid;selectedSecure=secure;
  var items=document.querySelectorAll('.network');items.forEach(function(el){el.classList.remove('selected')});
  var el=document.getElementById('net-'+ssid.replace(/'/g,''));
  if(el)el.classList.add('selected');
  document.getElementById('selected-name').textContent=ssid;
  var ps=document.getElementById('password-section');
  if(secure){ps.style.display='block';document.getElementById('save-btn').disabled=document.getElementById('password').value.length===0}
  else{ps.style.display='none';document.getElementById('save-btn').disabled=false}
}
document.getElementById('password').oninput=function(){
  document.getElementById('save-btn').disabled=this.value.length===0&&selectedSecure;
};
function save(){
  var pass=document.getElementById('password').value;
  document.getElementById('save-btn').disabled=true;
  document.getElementById('status').textContent='Saving...';
  fetch('/api/wifi/save',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"ssid":"'+selectedSSID+'","pass":"'+pass+'"}'})
  .then(function(r){return r.json()})
  .then(function(d){
    if(d.ok){document.getElementById('status').textContent='Connected! Restarting...'}
    else{document.getElementById('status').textContent='Failed';document.getElementById('save-btn').disabled=false}
  }).catch(function(){document.getElementById('status').textContent='Connection error';document.getElementById('save-btn').disabled=false});
}
scan();
</script>
</body>
</html>
)=====";

#endif
