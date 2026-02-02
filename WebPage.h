const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <title>Sara Samratah: The Ethical Choice</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <!-- Importing Cinzel for the Royal inscription look -->
  <link href="https://fonts.googleapis.com/css2?family=Cinzel:wght@400;700;900&family=Playfair+Display:wght@500;700&display=swap" rel="stylesheet">
  <style>
    :root { 
      /* DAYLIGHT / GOLDEN AGE PALETTE */
      --bg-core: #fffbf0; 
      --bg-gradient: radial-gradient(circle at center, #fffbf0 0%, #ffeec9 100%);
      
      --text-main: #4a0404;      /* Deep Brown/Red */
      --text-light: #fff;
      
      --saffron-main: #ff9933;   /* Dhwaj Saffron */
      --saffron-dark: #cc7a00;
      
      --gold-royal: #c5a009;     /* Metallic Gold */
      --gold-light: #fff8e1;
      
      --red-vermilion: #b71c1c;  /* Sindoor */
      --red-faint: rgba(183, 28, 28, 0.05);

      --success: #2e7d32;
      
      --shadow-card: 0 10px 25px rgba(183, 140, 20, 0.15);
      --border-ornate: 2px solid var(--gold-royal);
    }
    
    body { 
      font-family: 'Playfair Display', serif; 
      background: var(--bg-core);
      background-image: var(--bg-gradient);
      color: var(--text-main); 
      margin: 0; 
      padding: 20px; 
      text-align: center; 
      min-height: 100vh;
      box-sizing: border-box;
      border: 8px double var(--gold-royal); /* Ornate border around the screen */
    }

    h1 { 
      font-family: 'Cinzel', serif;
      font-size: 3rem;
      margin: 20px 0; 
      font-weight: 900; 
      letter-spacing: 2px; 
      color: var(--red-vermilion);
      text-transform: uppercase;
      text-shadow: 2px 2px 0px rgba(255, 215, 0, 0.5);
      position: relative;
      display: inline-block;
      padding-bottom: 15px;
      border-bottom: 3px solid var(--saffron-main);
    }
    
    /* Decorative elements under title */
    h1::after {
      content: '❖';
      position: absolute;
      bottom: -14px;
      left: 50%;
      transform: translateX(-50%);
      background: var(--bg-core);
      padding: 0 10px;
      color: var(--saffron-main);
      font-size: 1.5rem;
    }
    
    .status-container {
      margin: 30px auto;
      max-width: 700px;
      background: white;
      border-radius: 8px;
      border-left: 5px solid var(--saffron-main);
      border-right: 5px solid var(--saffron-main);
      box-shadow: 0 4px 12px rgba(0,0,0,0.1);
      padding: 20px;
      position: relative;
    }

    .status-label {
      text-transform: uppercase;
      font-size: 0.8rem;
      color: var(--gold-royal);
      font-weight: bold;
      letter-spacing: 2px;
      margin-bottom: 5px;
    }
    
    .status-text {
      font-family: 'Cinzel', serif;
      font-size: 1.5rem;
      color: var(--saffron-dark);
      font-weight: bold;
    }

    .grid { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); 
      gap: 30px; 
      max-width: 1000px; 
      margin: 0 auto 50px auto; 
    }
    
    /* White Marble Card Look */
    .card { 
      background: #ffffff; 
      padding: 30px 20px; 
      border-radius: 4px; /* Slightly squarer for architectural feel */
      box-shadow: var(--shadow-card); 
      border: 1px solid rgba(197, 160, 9, 0.3);
      position: relative;
      transition: transform 0.3s ease;
      overflow: hidden;
    }
    
    .card:hover { transform: translateY(-5px); box-shadow: 0 15px 35px rgba(183, 140, 20, 0.25); }
    
    /* Color accent tops */
    .card-acc-1 { border-top: 5px solid var(--saffron-main); }
    .card-acc-2 { border-top: 5px solid var(--gold-royal); }
    .card-acc-3 { border-top: 5px solid var(--success); }
    .card-acc-4 { border-top: 5px solid #5e35b1; } /* Deep Purple for Abhimanyu */

    .card h3 {
      font-family: 'Cinzel', serif;
      color: var(--text-main);
      font-size: 1.2rem;
      margin-top: 0;
    }

    .score { 
      font-family: 'Cinzel', serif;
      font-size: 4.5rem; 
      font-weight: 700; 
      margin: 10px 0; 
      color: var(--text-main);
    }
    
    .controls { 
      max-width: 650px; 
      margin: 0 auto; 
      padding: 30px; 
      background: white; 
      border-radius: 12px; 
      border: 2px dashed var(--gold-royal);
    }

    .controls h3 { margin-top:0; color: var(--red-vermilion); font-family: 'Cinzel', serif;}

    input { 
      padding: 12px 15px; 
      border-radius: 4px; 
      border: 2px solid var(--gold-royal); 
      width: 60%; 
      background: var(--bg-core); 
      color: var(--text-main); 
      font-family: inherit;
      font-size: 1rem;
      text-align: center;
      margin-right: 5px;
    }
    
    input:focus { outline: none; border-color: var(--saffron-main); box-shadow: 0 0 5px var(--saffron-main); }

    button { 
      padding: 12px 24px; 
      border-radius: 4px; 
      border: none; 
      font-family: 'Cinzel', serif;
      font-weight: 700; 
      cursor: pointer; 
      color: white; 
      text-transform: uppercase;
      box-shadow: 0 4px 6px rgba(0,0,0,0.2);
      transition: all 0.2s;
    }

    .btn-saffron { background-color: var(--saffron-main); }
    .btn-saffron:hover { background-color: var(--saffron-dark); transform: scale(1.05); }

    .btn-red { background-color: var(--red-vermilion); margin: 0 5px;}
    .btn-red:hover { filter: brightness(0.9); transform: scale(1.05); }

    table { 
      width: 100%; 
      max-width: 800px;
      margin: 30px auto; 
      border-collapse: collapse; 
      background: white; 
      border-radius: 8px; 
      overflow: hidden; 
      box-shadow: 0 4px 15px rgba(0,0,0,0.1);
      border: 1px solid var(--gold-royal);
    }
    th { 
      background: var(--saffron-main); 
      color: white; 
      padding: 15px; 
      font-family: 'Cinzel', serif;
      text-transform: uppercase;
      font-size: 0.9rem;
    }
    td { padding: 15px; border-bottom: 1px solid #eee; }
    tr:nth-child(even) { background-color: #fff8f0; }
    tr:hover { background-color: #ffe0b2; }

    .voted-yes { color: var(--success); font-weight: bold; }
    .voted-no { color: var(--red-vermilion); }

    @keyframes pulse {
      0% { opacity: 0.5; }
      50% { opacity: 1; transform: scale(1.02); }
      100% { opacity: 0.5; }
    }
    
    .active-sensor {
      color: var(--red-vermilion);
      animation: pulse 1.5s infinite;
    }
    
    .anon-active {
      background-color: var(--red-vermilion);
      color: white;
      padding: 5px 10px;
      border-radius: 4px;
      font-size: 0.9rem;
      display: inline-block;
      margin-left: 10px;
      animation: pulse 2s infinite;
      vertical-align: middle;
    }
    .anon-hidden { display: none; }
  </style>
</head>
<body>

  <h1>SARA SAMRATAH</h1>
  <div style="margin-top:-15px; color: var(--gold-royal); font-size: 0.9rem; letter-spacing: 2px;">THE ETHICAL VOTING INTERFACE</div>

  <div class="status-container">
    <div class="status-label">
      Sacred Gate Status 
      <span id="anonBadge" class="anon-active anon-hidden">ANONYMOUS MODE ACTIVE</span>
    </div>
    <div id="status" class="status-text">INITIATING DARBAR...</div>
  </div>

  <div class="grid">
    <div class="card card-acc-1">
      <h3 id="nA">Candidate A</h3>
      <div style="height: 2px; width: 50px; background: var(--saffron-main); margin: 0 auto;"></div>
      <div id="cntA" class="score" style="color: var(--saffron-main)">0</div>
    </div>
    <div class="card card-acc-2">
      <h3 id="nB">Candidate B</h3>
      <div style="height: 2px; width: 50px; background: var(--gold-royal); margin: 0 auto;"></div>
      <div id="cntB" class="score" style="color: var(--gold-royal)">0</div>
    </div>
    <div class="card card-acc-3">
      <h3 id="nC">Candidate C</h3>
      <div style="height: 2px; width: 50px; background: var(--success); margin: 0 auto;"></div>
      <div id="cntC" class="score" style="color: var(--success)">0</div>
    </div>
    <div class="card card-acc-4">
      <h3 id="nD">Candidate D</h3>
      <div style="height: 2px; width: 50px; background: #5e35b1; margin: 0 auto;"></div>
      <div id="cntD" class="score" style="color: #5e35b1">0</div>
    </div>
  </div>

  <div class="controls">
    <h3>REGISTRY CONTROL</h3>
    <div style="margin-bottom: 20px;">
      <input type="text" id="voterName" placeholder="Enter Citizen Name">
      <button class="btn-saffron" onclick="enroll()">REGISTER</button>
    </div>
    <hr style="border: 0; border-top: 1px dashed var(--gold-royal); margin: 20px 0;">
    <div>
      <button class="btn-red" onclick="resetSys()">RESET ELECTION</button>
      <button class="btn-red" onclick="clearDB()">WIPE RECORDS</button>
    </div>
  </div>

  <h3 style="color: var(--red-vermilion); font-family: 'Cinzel'; margin-top: 40px; text-transform:uppercase;">Citizen Roll</h3>
  <table id="userTable">
    <tr><th>ID</th><th>Citizen Name</th><th>Status</th><th>Action</th></tr>
  </table>

<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  
  function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
  }
  
  function onOpen(event) { console.log('Connected to Darbar'); }
  function onClose(event) { console.log('Connection Lost'); setTimeout(initWebSocket, 2000); }
  
  function onMessage(event) {
    var data = JSON.parse(event.data);
    var statusEl = document.getElementById('status');

    if (data.status) {
      statusEl.innerText = data.status;
      if(data.status.includes("Plz") || data.status.includes("Finger") || data.status.includes("Match")) {
         statusEl.classList.add('active-sensor');
      } else {
         statusEl.classList.remove('active-sensor');
      }
    }
    
    // Updates
    if (data.cntA !== undefined) document.getElementById('cntA').innerText = data.cntA;
    if (data.cntB !== undefined) document.getElementById('cntB').innerText = data.cntB;
    if (data.cntC !== undefined) document.getElementById('cntC').innerText = data.cntC;
    if (data.cntD !== undefined) document.getElementById('cntD').innerText = data.cntD;
    
    // Anonymous Mode Indicator
    if (data.anonMode !== undefined) {
      const badge = document.getElementById('anonBadge');
      if (data.anonMode) {
        badge.classList.remove('anon-hidden');
      } else {
        badge.classList.add('anon-hidden');
      }
    }

    if (data.nA) document.getElementById('nA').innerText = data.nA;
    if (data.nB) document.getElementById('nB').innerText = data.nB;
    if (data.nC) document.getElementById('nC').innerText = data.nC;
    if (data.nD) document.getElementById('nD').innerText = data.nD;
    
    if (data.users) renderTable(data.users);
    
    if (data.alert) alert(data.alert);
  }

  function renderTable(users) {
    const table = document.getElementById("userTable");
    while(table.rows.length > 1) { table.deleteRow(1); }
    
    users.forEach(u => {
      let row = table.insertRow();
      row.innerHTML = `
        <td style="color:var(--saffron-dark)"><b>${u.id}</b></td>
        <td style="font-size:1.1rem; color:var(--text-main);">${u.name}</td>
        <td class="${u.voted ? 'voted-yes' : 'voted-no'}">${u.voted ? '✓ CAST' : '• PENDING'}</td>
        <td><button style="padding:5px 10px; font-size:0.8rem;" class="btn-red" onclick="del(${u.id})">REMOVE</button></td>
      `;
    });
  }

  function enroll() {
    const name = document.getElementById('voterName').value;
    if(!name) { alert("Please provide a name."); return; }
    fetch('/enroll?name=' + encodeURIComponent(name));
  }
  function resetSys() { if(confirm("This will zero all votes. Proceed?")) fetch('/reset'); }
  function clearDB() { if(confirm("CRITICAL: Wipe entire database?")) fetch('/wipe'); }
  function del(id) { if(confirm("Remove ID " + id + "?")) fetch('/delete?id=' + id); }

  window.onload = initWebSocket;
</script>
</body>
</html>
)rawliteral";