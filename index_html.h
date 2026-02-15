const char indexHtml[] PROGMEM = R"IndexHTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Calendar</title>
  <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@fortawesome/fontawesome-free@6.4.0/css/all.min.css">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <style>
    * { font-family: 'Inter', sans-serif; }
    :root {
      --bg-primary: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      --bg-secondary: rgba(255, 255, 255, 0.1);
      --glass-bg: rgba(255, 255, 255, 0.15);
      --glass-border: rgba(255, 255, 255, 0.2);
      --text-primary: #ffffff;
      --text-secondary: rgba(255, 255, 255, 0.8);
      --shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
    }
    [data-theme="light"] {
      --bg-primary: linear-gradient(135deg, #74b9ff 0%, #0984e3 100%);
      --bg-secondary: rgba(0, 0, 0, 0.05);
      --glass-bg: rgba(255, 255, 255, 0.25);
      --glass-border: rgba(255, 255, 255, 0.4);
      --text-primary: #2d3436;
      --text-secondary: rgba(45, 52, 54, 0.7);
      --shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.2);
    }
    body { background: var(--bg-primary); color: var(--text-primary); min-height: 100vh; backdrop-filter: blur(10px); }
    .glass {
      background: var(--glass-bg);
      backdrop-filter: blur(16px);
      border: 1px solid var(--glass-border);
      box-shadow: var(--shadow);
    }
    .calendar-grid { display: grid; grid-template-columns: repeat(7, 1fr); gap: 8px; margin-top: 20px; }
    .calendar-day {
      height: 120px;
      border-radius: 12px;
      padding: 8px;
      cursor: pointer;
      transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
      position: relative;
      overflow: hidden;
      display: flex;
      flex-direction: column;
    }
    .calendar-day:hover { transform: translateY(-2px); box-shadow: 0 12px 40px 0 rgba(31, 38, 135, 0.5); }
    .day-number { font-weight: 600; font-size: 16px; margin-bottom: 6px; min-height: 24px; }
    .today {
      background: linear-gradient(135deg, rgba(116, 185, 255, 0.3) 0%, rgba(9, 132, 227, 0.3) 100%);
      border: 2px solid rgba(116, 185, 255, 0.6);
    }
    .events-container { flex: 1; overflow: hidden; position: relative; }
    .event-item {
      display: flex; align-items: center; gap: 6px; padding: 3px 6px; margin-bottom: 2px; border-radius: 6px;
      font-size: 10px; font-weight: 500; transition: all 0.2s ease; cursor: pointer;
      overflow: hidden; white-space: nowrap; text-overflow: ellipsis; max-width: 100%;
    }
    .event-work { background: linear-gradient(135deg, rgba(255, 159, 159, 0.9) 0%, rgba(255, 107, 107, 0.9) 100%); }
    .event-personal { background: linear-gradient(135deg, rgba(159, 214, 255, 0.9) 0%, rgba(74, 144, 226, 0.9) 100%); }
    .event-other { background: linear-gradient(135deg, rgba(209, 255, 159, 0.9) 0%, rgba(130, 204, 221, 0.9) 100%); }
    .event-work, .event-personal, .event-other, .event-work *, .event-personal *, .event-other * { color: #000000 !important; }
    .event-time { font-family: 'Courier New', monospace; font-weight: 600; font-size: 9px; }
    .show-more {
      background: linear-gradient(135deg, rgba(99, 102, 241, 0.8) 0%, rgba(139, 69, 19, 0.8) 100%);
      color: white; font-size: 9px; padding: 2px 4px; border-radius: 4px; text-align: center; cursor: pointer; transition: all 0.2s ease; margin-top: 2px;
    }
    .modal {
      background: var(--glass-bg); backdrop-filter: blur(20px); border: 1px solid var(--glass-border); border-radius: 20px;
      box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.25); max-width: 500px; width: 90%; max-height: 80vh; overflow-y: auto;
    }
    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); border: none; border-radius: 10px;
      padding: 10px 20px; color: white; font-weight: 600; cursor: pointer; transition: all 0.3s ease;
      box-shadow: 0 4px 15px 0 rgba(102, 126, 234, 0.4);
    }
    .btn-primary:hover { transform: translateY(-2px); box-shadow: 0 8px 25px 0 rgba(102, 126, 234, 0.6); }
    .btn-secondary {
      background: var(--glass-bg); border: 1px solid var(--glass-border); border-radius: 10px;
      padding: 10px 20px; color: var(--text-primary); font-weight: 500; cursor: pointer; transition: all 0.3s ease; backdrop-filter: blur(10px);
    }
    .form-input {
      background: var(--glass-bg); border: 1px solid var(--glass-border); border-radius: 10px;
      padding: 10px 15px; color: var(--text-primary); backdrop-filter: blur(10px); width: 100%; transition: all 0.3s ease;
    }
    .form-input:focus { outline: none; border-color: #667eea; box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1); }
    .theme-toggle {
      position: fixed; top: 20px; right: 20px; width: 50px; height: 50px; border-radius: 50%;
      display: flex; align-items: center; justify-content: center; background: var(--glass-bg); cursor: pointer;
      border: 1px solid var(--glass-border); transition: all 0.3s ease; backdrop-filter: blur(10px); z-index: 1000;
    }
    .logout-btn {
      position: fixed; top: 20px; left: 20px; padding: 10px 20px; border-radius: 20px;
      background: rgba(255,50,50,0.5); color: white; cursor: pointer; text-decoration: none;
      display: flex; align-items: center; gap: 8px; font-weight: bold; border: 1px solid rgba(255,255,255,0.2);
    }
  </style>
</head>
<body>
  <button class="theme-toggle" onclick="toggleTheme()"><i class="fas fa-moon" id="themeIcon"></i></button>
  <div style="position: fixed; top: 20px; left: 20px; display: flex; gap: 10px; z-index: 1000;">
      <a href="/logout" class="logout-btn" style="position: static;"><i class="fas fa-sign-out-alt"></i> Logout</a>
      <a href="/admin" class="logout-btn" style="position: static; background: rgba(50, 50, 255, 0.5);"><i class="fas fa-user-shield"></i> Admin</a>
  </div>

  <div class="container mx-auto px-4 py-8">
    <div class="text-center mb-8">
      <h1 class="text-4xl font-bold mb-4">ESP32 Calendar</h1>
      
      <!-- Calendar Selector -->
      <div id="calSelectorContainer" class="hidden mt-4 flex justify-center">
          <select id="calendarSelector" onchange="changeCalendar()" class="p-2 rounded bg-white bg-opacity-20 border border-white border-opacity-30 outline-none text-black">
              <!-- Options loaded via JS -->
          </select>
      </div>
    </div>
    
    <div class="glass rounded-xl p-6 mb-6 flex justify-between items-center">
        <button onclick="prevMonth()" class="btn-secondary px-4 py-2"><i class="fas fa-chevron-left"></i></button>
        <h2 class="text-2xl font-bold" id="monthYear"></h2>
        <button onclick="nextMonth()" class="btn-secondary px-4 py-2"><i class="fas fa-chevron-right"></i></button>
    </div>
    <div class="glass rounded-xl p-6">
      <div class="grid grid-cols-7 gap-2 mb-4 text-center font-bold">
        <div>Sun</div><div>Mon</div><div>Tue</div><div>Wed</div><div>Thu</div><div>Fri</div><div>Sat</div>
      </div>
      <div class="calendar-grid" id="calendar"></div>
    </div>
  </div>

  <div id="modalOverlay" class="fixed inset-0 bg-black bg-opacity-50 z-50 hidden items-center justify-center backdrop-blur-sm" onclick="if(event.target===this) closeModal()">
    <div class="modal p-6">
      <div class="flex justify-between items-center mb-6">
        <h3 class="text-xl font-bold flex items-center gap-2">
          <i class="fas fa-calendar-day"></i>
          Events for <span id="modalDate" class="text-blue-400"></span>
        </h3>
        <button onclick="closeModal()" class="text-2xl opacity-60 hover:opacity-100 transition-opacity"><i class="fas fa-times"></i></button>
      </div>
      
      <!-- Edit Controls (Only for Owner) -->
      <div id="editControls" class="mb-6 p-4 rounded-xl" style="background: var(--bg-secondary);">
        <h4 class="font-semibold mb-4 flex items-center gap-2">
          <i class="fas fa-plus"></i>
          Add New Event
        </h4>
        
        <div class="grid grid-cols-1 md:grid-cols-3 gap-4 mb-4">
          <div>
            <label class="block text-sm font-medium mb-2">Time</label>
            <input type="time" id="eventTime" class="form-input" value="12:00">
          </div>
          <div class="md:col-span-2">
            <label class="block text-sm font-medium mb-2">Event Description</label>
            <input type="text" id="eventText" class="form-input" placeholder="Enter event description...">
          </div>
        </div>
        
        <div class="flex gap-4 items-end">
          <div class="flex-1">
            <label class="block text-sm font-medium mb-2">Category</label>
            <select id="eventCategory" class="form-input">
              <option value="Work">💼 Work</option>
              <option value="Personal">🏠 Personal</option>
              <option value="Other">🎯 Other</option>
            </select>
          </div>
          <button onclick="addEvent()" class="btn-primary flex items-center gap-2">
            <i class="fas fa-plus"></i>
            Add Event
          </button>
        </div>
      </div>
      
      <div id="readOnlyMsg" class="hidden text-center text-yellow-300 font-bold mb-4">Viewing Mode Only</div>

      <div id="eventsListSection">
        <h4 class="font-semibold mb-4 flex items-center gap-2">
          <i class="fas fa-list"></i>
          Today's Events
        </h4>
        <div id="eventsContainer" class="space-y-2"></div>
      </div>
    </div>
  </div>

  <!-- Custom Modal -->
  <div id="customModal" class="fixed inset-0 z-[100] flex items-center justify-center bg-black bg-opacity-70 hidden opacity-0 transition-opacity duration-300" style="z-index: 10000;">
      <div class="glass p-6 max-w-sm w-full mx-4 transform scale-95 transition-transform duration-300" id="customModalContent">
          <h3 id="customModalTitle" class="text-xl font-bold mb-4 text-white"></h3>
          <p id="customModalMessage" class="mb-6 text-gray-300 font-medium"></p>
          <div class="flex justify-end gap-3" id="customModalActions"></div>
      </div>
  </div>

  <script>
    let currentDate = new Date();
    let events = {};
    let selectedDate = '';
    let currentUser = '';
    let currentViewUser = '';
    const MAX_VISIBLE_EVENTS = 3;

    document.addEventListener('DOMContentLoaded', () => { 
        initTheme(); 
        loadAccessibleCalendars();
    });

    function initTheme() {
      const t = localStorage.getItem('theme') || 'dark';
      document.documentElement.setAttribute('data-theme', t);
      const icon = document.getElementById('themeIcon');
      if(icon) icon.className = t==='dark'?'fas fa-sun':'fas fa-moon';
    }

    function toggleTheme() {
      const t = document.documentElement.getAttribute('data-theme')==='dark'?'light':'dark';
      document.documentElement.setAttribute('data-theme', t);
      localStorage.setItem('theme', t);
      initTheme();
    }

    function loadAccessibleCalendars() {
        fetch('/accessible_calendars')
        .then(res => res.json())
        .then(users => {
            if(Array.isArray(users) && users.length > 0) {
                 currentUser = users[0];
                 currentViewUser = currentUser;
                 const sel = document.getElementById('calendarSelector');
                 sel.innerHTML = '';
                 users.forEach(u => {
                     const opt = document.createElement('option');
                     opt.value = u;
                     opt.innerText = (u === currentUser) ? "My Calendar" : u + "'s Calendar";
                     opt.className = "text-black";
                     sel.appendChild(opt);
                 });
                 if(users.length > 1) document.getElementById('calSelectorContainer').classList.remove('hidden');
            }
            loadEvents();
        })
        .catch(e => {
            console.error("Failed to load calendars", e);
            loadEvents();
        });
    }

    function changeCalendar() {
        currentViewUser = document.getElementById('calendarSelector').value;
        loadEvents();
    }

    function loadEvents() {
      let url = '/events';
      if(currentViewUser) url += '?user=' + currentViewUser;
      fetch(url)
      .then(r => r.json())
      .then(d => { 
        events = (d && typeof d === 'object') ? d : {}; 
        renderCalendar(); 
      })
      .catch(e => {
        console.error("Load events error", e);
        events = {};
        renderCalendar();
      });
    }

    function saveEvents() {
      if(currentViewUser && currentViewUser !== currentUser) return;
      const formData = new FormData();
      formData.append('plain', JSON.stringify(events));
      fetch('/events', { method: 'POST', body: formData })
        .then(res => { if(!res.ok) showCustomModal('Error', 'Failed to save events to ESP32', false); })
        .catch(e => showCustomModal('Error', 'Network error while saving', false));
    }

    function renderCalendar() {
      const cal = document.getElementById("calendar");
      if(!cal) return;
      cal.innerHTML = '';
      const year = currentDate.getFullYear();
      const month = currentDate.getMonth();
      document.getElementById("monthYear").innerText = new Date(year, month).toLocaleString('default', {month:'long', year:'numeric'});
      
      const firstDay = new Date(year, month, 1).getDay();
      const daysInMonth = new Date(year, month + 1, 0).getDate();

      for(let i=0; i<firstDay; i++) cal.innerHTML += '<div></div>';

      for(let d=1; d<=daysInMonth; d++) {
        const dateKey = `${year}-${month+1}-${d}`;
        const dayDiv = document.createElement('div');
        dayDiv.className = `calendar-day glass ${new Date().toDateString() === new Date(year, month, d).toDateString() ? 'today' : ''}`;
        dayDiv.onclick = () => openModal(dateKey);
        
        dayDiv.innerHTML = `<div class="day-number">${d}</div>`;
        
        const eventsContainer = document.createElement('div');
        eventsContainer.className = 'events-container';
        
        if(events[dateKey] && Array.isArray(events[dateKey])) {
            events[dateKey].slice(0, MAX_VISIBLE_EVENTS).forEach(ev => {
                const icon = getIconForCategory(ev.category);
                eventsContainer.innerHTML += `
                  <div class="event-item event-${ev.category.toLowerCase()}">
                    <span style="font-size:8px">${icon}</span>
                    <span class="event-time">${ev.time}</span>
                    <span>${ev.text}</span>
                  </div>`;
            });
            if(events[dateKey].length > MAX_VISIBLE_EVENTS) {
                eventsContainer.innerHTML += `<div class="show-more">+${events[dateKey].length - MAX_VISIBLE_EVENTS} more</div>`;
            }
        }
        dayDiv.appendChild(eventsContainer);
        cal.appendChild(dayDiv);
      }
    }

    function getIconForCategory(category) {
      const icons = { 'Work': '💼', 'Personal': '🏠', 'Other': '🎯' };
      return icons[category] || '📅';
    }

    function openModal(date) {
      selectedDate = date;
      const d = new Date(date);
      document.getElementById('modalDate').innerText = d.toLocaleDateString('en-US', { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' });
      
      const isOwner = (!currentViewUser || currentViewUser === currentUser);
      if(isOwner) {
          document.getElementById('editControls').classList.remove('hidden');
          document.getElementById('readOnlyMsg').classList.add('hidden');
      } else {
          document.getElementById('editControls').classList.add('hidden');
          document.getElementById('readOnlyMsg').classList.remove('hidden');
      }

      updateEventList();
      document.getElementById('modalOverlay').classList.remove('hidden');
      document.getElementById('modalOverlay').classList.add('flex');
    }

    function closeModal() {
      document.getElementById('modalOverlay').classList.add('hidden');
      document.getElementById('modalOverlay').classList.remove('flex');
    }

    function addEvent() {
      const text = document.getElementById('eventText').value;
      const time = document.getElementById('eventTime').value;
      const category = document.getElementById('eventCategory').value;
      if(!text) return;
      if(!events[selectedDate]) events[selectedDate] = [];
      events[selectedDate].push({text, time, category});
      events[selectedDate].sort((a,b) => a.time.localeCompare(b.time));
      saveEvents();
      document.getElementById('eventText').value = '';
      updateEventList();
      renderCalendar();
    }

    function deleteEvent(idx) {
        showCustomModal('Delete Event', 'Are you sure you want to delete this event?', true, () => {
            if(events[selectedDate] && events[selectedDate][idx]) {
                events[selectedDate].splice(idx, 1);
                if(events[selectedDate].length === 0) delete events[selectedDate];
                saveEvents();
                updateEventList();
                renderCalendar();
            }
        });
    }

    // --- Custom Modal Logic ---
    function showCustomModal(title, message, isConfirm, onConfirm) {
        const modal = document.getElementById('customModal');
        const content = document.getElementById('customModalContent');
        
        document.getElementById('customModalTitle').innerText = title;
        document.getElementById('customModalTitle').className = `text-xl font-bold mb-4 ${title === 'Error' || title.includes('Delete') ? 'text-red-400' : 'text-green-400'}`;
        document.getElementById('customModalMessage').innerText = message;
        
        const actions = document.getElementById('customModalActions');
        actions.innerHTML = '';

        if (isConfirm) {
            const btnCancel = document.createElement('button');
            btnCancel.className = 'px-4 py-2 rounded bg-gray-600 hover:bg-gray-500 text-white font-medium transition';
            btnCancel.innerText = 'Cancel';
            btnCancel.onclick = closeCustomModal;
            
            const btnConfirm = document.createElement('button');
            btnConfirm.className = 'px-4 py-2 rounded bg-red-600 hover:bg-red-500 text-white font-bold shadow-lg transition';
            btnConfirm.innerText = 'Confirm';
            btnConfirm.onclick = () => { closeCustomModal(); if(onConfirm) onConfirm(); };
            
            actions.appendChild(btnCancel);
            actions.appendChild(btnConfirm);
        } else {
            const btnOk = document.createElement('button');
            btnOk.className = 'px-4 py-2 rounded bg-blue-600 hover:bg-blue-500 text-white font-bold shadow-lg transition';
            btnOk.innerText = 'OK';
            btnOk.onclick = closeCustomModal;
            actions.appendChild(btnOk);
        }

        modal.classList.remove('hidden');
        setTimeout(() => {
            modal.classList.remove('opacity-0');
            content.classList.remove('scale-95');
            content.classList.add('scale-100');
        }, 10);
    }

    function closeCustomModal() {
        const modal = document.getElementById('customModal');
        const content = document.getElementById('customModalContent');
        modal.classList.add('opacity-0');
        content.classList.remove('scale-100');
        content.classList.add('scale-95');
        setTimeout(() => {
            modal.classList.add('hidden');
        }, 300);
    }

    function updateEventList() {
        const container = document.getElementById('eventsContainer');
        container.innerHTML = '';
        const isOwner = (!currentViewUser || currentViewUser === currentUser);

        if(events[selectedDate] && Array.isArray(events[selectedDate])) {
            events[selectedDate].forEach((ev, idx) => {
                let delBtn = '';
                if(isOwner) {
                    delBtn = `<button onclick="deleteEvent(${idx})" class="text-red-500 hover:text-red-700 transition-colors p-2"><i class="fas fa-trash"></i></button>`;
                }
                const icon = getIconForCategory(ev.category);
                container.innerHTML += `
                <div class="flex items-center justify-between p-3 rounded-lg event-work" style="background:var(--bg-secondary); color:white !important;">
                  <div class="flex items-center gap-3">
                    <span class="text-lg">${icon}</span>
                    <div>
                      <div class="font-semibold" style="color:var(--text-primary) !important;">${ev.text}</div>
                      <div class="text-sm opacity-75" style="color:var(--text-primary) !important;">${ev.time} • ${ev.category}</div>
                    </div>
                  </div>
                  ${delBtn}
                </div>`;
            });
        } else {
            container.innerHTML = '<div class="text-center py-8 opacity-60"><p>No events scheduled</p></div>';
        }
    }
    
    function prevMonth() { currentDate.setMonth(currentDate.getMonth()-1); renderCalendar(); }
    function nextMonth() { currentDate.setMonth(currentDate.getMonth()+1); renderCalendar(); }
  </script>
</body>
</html>
)IndexHTML";