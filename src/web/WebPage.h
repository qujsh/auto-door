#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title>智能自动门 V3</title>
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
.btn:disabled{background:#252535;color:#666;cursor:not-allowed;opacity:.65}
.btn-row{display:flex;gap:8px;margin-top:10px}
.btn-row .btn{flex:1}
.slider-wrap{margin:4px 0}
input[type=range]{width:100%;height:6px;-webkit-appearance:none;appearance:none;background:#1e3a5f;border-radius:3px;outline:none}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;background:#533483;border-radius:50%;cursor:pointer}
input[type=range]:disabled{opacity:.35;cursor:not-allowed}
.angle-val{text-align:center;font-size:28px;font-weight:700;margin:4px 0 2px}
.setting-grid{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:8px}
.setting-grid label{font-size:12px;color:#888}
.setting-grid input{width:100%;margin-top:4px;padding:8px;border:1px solid #30304a;border-radius:6px;background:#101020;color:#e0e0e0}
.setting-note{font-size:11px;color:#777;margin-top:8px;line-height:1.5}
.mode-row{display:flex;gap:20px;align-items:center;padding:4px 0}
.mode-row label{cursor:pointer;font-size:15px}
.mode-row input[type=radio]{accent-color:#533483;width:16px;height:16px;cursor:pointer}
#log{background:#0a0a14;border-radius:8px;padding:8px;font-size:11px;max-height:120px;overflow-y:auto;font-family:monospace;margin-top:10px}
.log-line{padding:2px 0;border-bottom:1px solid #141428;color:#888}
</style>
</head>
<body>
<h1>智能自动门 V3</h1>
<div class="sub">ESP32-C3 自动门控制系统</div>

<div class="card">
<div class="row"><span class="lbl">Wi-Fi 状态</span><span id="wifi" class="val g">-</span></div>
<div class="row"><span class="lbl">设备 IP</span><span id="ip" class="val">-</span></div>
<div class="row"><span class="lbl">信号强度</span><span id="rssi" class="val">-</span></div>
</div>

<div class="card">
<div class="row"><span class="lbl">当前距离</span><span id="dist" class="val">-</span></div>
<div class="row"><span class="lbl">环境基线</span><span id="base" class="val">-</span></div>
<div class="row"><span class="lbl">距离变化</span><span id="diff" class="val">-</span></div>
<div class="row"><span class="lbl">检测到目标</span><span id="detect" class="val r">-</span></div>
<div class="row"><span class="lbl">确认有人</span><span id="present" class="val r">-</span></div>
<div class="row"><span class="lbl">门状态</span><span id="door" class="val">-</span></div>
<div class="row"><span class="lbl">舵机角度</span><span id="servo" class="val">-</span></div>
</div>

<div class="card">
<div class="lbl" style="margin-bottom:6px">运行模式</div>
<div class="mode-row">
<label><input type="radio" name="mode" value="auto" onchange="setMode('auto')"> 自动</label>
<label><input type="radio" name="mode" value="manual" onchange="setMode('manual')"> 手动</label>
</div>
</div>

<div class="card">
<div class="lbl">手动舵机角度</div>
<div class="angle-val"><span id="angleDisp">0</span>&#176;</div>
<div class="slider-wrap"><input type="range" id="slider" class="manual-control" min="0" max="180" value="0" oninput="onSlider()" onchange="onSlider()" disabled></div>
<div class="btn-row">
<button class="btn manual-control" onclick="setServo(currentInitialAngle)" disabled>关门</button>
<button class="btn primary manual-control" onclick="setServo(currentOpenAngle)" disabled>开门</button>
<button class="btn" onclick="calibrate()">重新标定</button>
</div>
</div>

<div class="card">
<div class="lbl">自动门参数</div>
<div class="row"><span class="lbl">计算后的开门角度</span><span id="openAngle" class="val">90°</span></div>
<div class="setting-grid">
<label>初始/关门角度（0～180°）<input id="initialInput" class="manual-control" type="number" min="0" max="180" value="0" oninput="previewOpenAngle()" disabled></label>
<label>相对转动角度（-180～180°）<input id="rotationInput" class="manual-control" type="number" min="-180" max="180" value="90" oninput="previewOpenAngle()" disabled></label>
<label>触发距离差（0.1～200cm）<input id="thresholdInput" class="manual-control" type="number" min="0.1" max="200" step="0.1" value="2.5" disabled></label>
</div>
<button class="btn primary manual-control" style="width:100%;margin-top:10px" onclick="saveSettings()" disabled>保存自动门参数</button>
<div class="setting-note">初始角度和“初始角度 + 相对转动角度”都必须在 0～180°。保存后舵机会移动到新的初始/关门角度；正负转动角度用于选择开门方向。</div>
</div>

<div id="log"></div>

<script>
var ip='',autoAngle=0,currentMode='AUTO',settingsLoaded=false,currentInitialAngle=0,currentOpenAngle=90;
function onSlider(){var v=document.getElementById('slider').value;document.getElementById('angleDisp').textContent=v;setServo(v)}
function setServo(a){if(currentMode!=='MANUAL')return;document.getElementById('slider').value=a;document.getElementById('angleDisp').textContent=a;fetch('/api/servo?angle='+a)}
function setMode(m){fetch('/api/mode',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"mode":"'+m+'"}'}).then(update)}
function setManualControlsEnabled(enabled){document.querySelectorAll('.manual-control').forEach(function(el){el.disabled=!enabled})}
function calibrate(){fetch('/api/calibrate',{method:'POST'}).then(function(r){return r.json()}).then(function(d){if(d.baseline)document.getElementById('base').textContent=d.baseline.toFixed(2)+' cm'})}
function previewOpenAngle(){var a=Number(document.getElementById('initialInput').value),r=Number(document.getElementById('rotationInput').value),o=a+r,e=document.getElementById('openAngle');e.textContent=(a>=0&&a<=180&&r>=-180&&r<=180&&o>=0&&o<=180)?o+'°':'超出范围';e.className=(o>=0&&o<=180)?'val':'val r'}
function saveSettings(){
  if(currentMode!=='MANUAL')return;
  var initial=Number(document.getElementById('initialInput').value);
  var rotation=document.getElementById('rotationInput').value;
  var threshold=document.getElementById('thresholdInput').value;
  var open=initial+Number(rotation);
  if(initial<0||initial>180||open<0||open>180){alert('初始角度或计算后的开门角度超出 0～180°');return}
  fetch('/api/settings?initial='+encodeURIComponent(initial)+'&rotation='+encodeURIComponent(rotation)+'&threshold='+encodeURIComponent(threshold),{method:'POST'})
    .then(function(r){if(!r.ok)throw new Error('save failed');return r.json()})
    .then(function(){settingsLoaded=false;update()})
}
function addLog(m){var l=document.getElementById('log'),t=new Date().toLocaleTimeString();l.innerHTML+='<div class="log-line">'+t+' '+m+'</div>';l.scrollTop=l.scrollHeight;while(l.children.length>50)l.removeChild(l.firstChild)}
function update(){
  fetch('/api/status').then(function(r){return r.json()}).then(function(d){
    var w=document.getElementById('wifi');w.textContent=d.wifi?'已连接':'未连接';w.className='val '+(d.wifi?'g':'r');
    document.getElementById('ip').textContent=d.ip||'-';
    document.getElementById('rssi').textContent=d.rssi!=null?d.rssi+' dBm':'-';
    if(d.distance>=0)document.getElementById('dist').textContent=d.distance.toFixed(2)+' cm';
    document.getElementById('base').textContent=d.baseline.toFixed(2)+' cm';
    document.getElementById('diff').textContent=d.diff.toFixed(2)+' cm';
    var dt=document.getElementById('detect');dt.textContent=d.detect?'是':'否';dt.className='val '+(d.detect?'g':'r');
    var pr=document.getElementById('present');pr.textContent=d.present?'是':'否';pr.className='val '+(d.present?'g':'r');
    var dr=document.getElementById('door');
    dr.textContent=d.door==='OPEN'?'已打开':(d.door==='WAIT_CLOSE'?'等待关门':(d.door==='CLOSED'?'已关闭':'未知'));
    if(d.door==='OPEN')dr.className='val g';
    else if(d.door==='WAIT_CLOSE')dr.className='val y';
    else dr.className='val';
    document.getElementById('servo').textContent=d.servo+'°';
    currentInitialAngle=d.initialAngle;currentOpenAngle=d.openAngle;
    document.getElementById('openAngle').textContent=currentOpenAngle+'°';
    document.getElementById('openAngle').className='val';
    if(!settingsLoaded){document.getElementById('initialInput').value=d.initialAngle;document.getElementById('rotationInput').value=d.rotationAngle;document.getElementById('thresholdInput').value=d.distanceThreshold;settingsLoaded=true}
    currentMode=d.mode;
    setManualControlsEnabled(d.mode==='MANUAL');
    if(d.mode==='AUTO'){document.querySelector('input[value=auto]').checked=true}
    else{document.querySelector('input[value=manual]').checked=true}
    if(d.mode!=='MANUAL'){autoAngle=d.servo;document.getElementById('slider').value=autoAngle;document.getElementById('angleDisp').textContent=autoAngle}
  }).catch(function(e){document.getElementById('wifi').textContent='离线';document.getElementById('wifi').className='val r'})
}
setInterval(update,500);
update();
</script>
</body>
</html>
)=====";

#endif
