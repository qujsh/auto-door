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

#endif
