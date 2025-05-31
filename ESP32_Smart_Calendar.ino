#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "TEST";
const char* password = "TEST";

// Create WebServer object on port 80
WebServer server(80);

// File paths
const char* eventsFilePath = "/calendar_events.json";
const char* indexHtmlPath = "/index.html";

// Buffer for JSON document
StaticJsonDocument<8192> jsonDoc; // Adjust size as needed

// Function to initialize SPIFFS with better error reporting
bool initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("ERROR: SPIFFS Mount Failed");
    return false;
  }
  
  Serial.println("SPIFFS mounted successfully");
  
  // List files in SPIFFS to verify it's working
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  Serial.println("SPIFFS files:");
  while(file) {
    Serial.print("  ");
    Serial.print(file.name());
    Serial.print(" (");
    Serial.print(file.size());
    Serial.println(" bytes)");
    file = root.openNextFile();
  }

  // Check if events file exists, create it if it doesn't
  if (!SPIFFS.exists(eventsFilePath)) {
    File file = SPIFFS.open(eventsFilePath, FILE_WRITE);
    if (file) {
      file.println("{}");
      file.close();
      Serial.println("Created empty events file");
    } else {
      Serial.println("Failed to create events file");
      return false;
    }
  }
  
  // Create the HTML file if it doesn't exist
  if (!SPIFFS.exists(indexHtmlPath)) {
    return createHtmlFile();
  }
  
  return true;
}

// Function to create the HTML file in SPIFFS with better error handling
bool createHtmlFile() {
  File file = SPIFFS.open(indexHtmlPath, FILE_WRITE);
  if (!file) {
    Serial.println("ERROR: Failed to create HTML file");
    return false;
  }
  
// HTML content
  file.print(R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Calendar</title>
  <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@fortawesome/fontawesome-free@6.4.0/css/all.min.css">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <style>
    * {
      font-family: 'Inter', sans-serif;
    }

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

    body {
      background: var(--bg-primary);
      color: var(--text-primary);
      min-height: 100vh;
      backdrop-filter: blur(10px);
    }

    .glass {
      background: var(--glass-bg);
      backdrop-filter: blur(16px);
      border: 1px solid var(--glass-border);
      box-shadow: var(--shadow);
    }

    .calendar-grid {
      display: grid;
      grid-template-columns: repeat(7, 1fr);
      gap: 8px;
      margin-top: 20px;
    }

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

    .calendar-day:hover {
      transform: translateY(-2px);
      box-shadow: 0 12px 40px 0 rgba(31, 38, 135, 0.5);
    }

    .day-number {
      font-weight: 600;
      font-size: 16px;
      margin-bottom: 6px;
      min-height: 24px;
    }

    .today {
      background: linear-gradient(135deg, rgba(116, 185, 255, 0.3) 0%, rgba(9, 132, 227, 0.3) 100%);
      border: 2px solid rgba(116, 185, 255, 0.6);
    }

    .events-container {
      flex: 1;
      overflow: hidden;
      position: relative;
    }

    .event-item {
      display: flex;
      align-items: center;
      gap: 6px;
      padding: 3px 6px;
      margin-bottom: 2px;
      border-radius: 6px;
      font-size: 10px;
      font-weight: 500;
      transition: all 0.2s ease;
      cursor: pointer;
      overflow: hidden;
      white-space: nowrap;
      text-overflow: ellipsis;
      max-width: 100%;
    }

    .event-item:hover {
      transform: scale(1.02);
      z-index: 10;
      position: relative;
    }

    .event-work {
      background: linear-gradient(135deg, rgba(255, 159, 159, 0.9) 0%, rgba(255, 107, 107, 0.9) 100%);
    }

    .event-personal {
      background: linear-gradient(135deg, rgba(159, 214, 255, 0.9) 0%, rgba(74, 144, 226, 0.9) 100%);
    }

    .event-other {
      background: linear-gradient(135deg, rgba(209, 255, 159, 0.9) 0%, rgba(130, 204, 221, 0.9) 100%);
    }

    .event-work,
    .event-personal,
    .event-other,
    .event-work *,
    .event-personal *,
    .event-other * {
      color: #000000 !important;
    }

    .event-time {
      font-family: 'Courier New', monospace;
      font-weight: 600;
      font-size: 9px;
    }

    .show-more {
      background: linear-gradient(135deg, rgba(99, 102, 241, 0.8) 0%, rgba(139, 69, 19, 0.8) 100%);
      color: white;
      font-size: 9px;
      padding: 2px 4px;
      border-radius: 4px;
      text-align: center;
      cursor: pointer;
      transition: all 0.2s ease;
      margin-top: 2px;
    }

    .show-more:hover {
      transform: scale(1.05);
      background: linear-gradient(135deg, rgba(99, 102, 241, 1) 0%, rgba(139, 69, 19, 1) 100%);
    }

    .modal {
      background: var(--glass-bg);
      backdrop-filter: blur(20px);
      border: 1px solid var(--glass-border);
      border-radius: 20px;
      box-shadow: 0 25px 50px -12px rgba(0, 0, 0, 0.25);
      max-width: 500px;
      width: 90%;
      max-height: 80vh;
      overflow-y: auto;
    }

    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border: none;
      border-radius: 10px;
      padding: 10px 20px;
      color: white;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 4px 15px 0 rgba(102, 126, 234, 0.4);
    }

    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 25px 0 rgba(102, 126, 234, 0.6);
    }

    .btn-secondary {
      background: var(--glass-bg);
      border: 1px solid var(--glass-border);
      border-radius: 10px;
      padding: 10px 20px;
      color: var(--text-primary);
      font-weight: 500;
      cursor: pointer;
      transition: all 0.3s ease;
      backdrop-filter: blur(10px);
    }

    .btn-secondary:hover {
      background: var(--glass-border);
      transform: translateY(-1px);
    }

    .form-input {
      background: var(--glass-bg);
      border: 1px solid var(--glass-border);
      border-radius: 10px;
      padding: 10px 15px;
      color: var(--text-primary);
      backdrop-filter: blur(10px);
      width: 100%;
      transition: all 0.3s ease;
    }

    .form-input:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }

    .theme-toggle {
      position: fixed;
      top: 20px;
      right: 20px;
      background: var(--glass-bg);
      border: 1px solid var(--glass-border);
      border-radius: 50%;
      width: 50px;
      height: 50px;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      transition: all 0.3s ease;
      backdrop-filter: blur(10px);
      z-index: 1000;
    }

    .theme-toggle:hover {
      transform: scale(1.1);
      background: var(--glass-border);
    }

    .event-tooltip {
      position: absolute;
      background: rgba(0, 0, 0, 0.9);
      color: white;
      padding: 8px 12px;
      border-radius: 8px;
      font-size: 12px;
      z-index: 1000;
      pointer-events: none;
      transform: translateX(-50%);
      opacity: 0;
      transition: opacity 0.3s ease;
      backdrop-filter: blur(10px);
    }

    .event-tooltip.show {
      opacity: 1;
    }

    .category-icon {
      font-size: 8px;
      width: 12px;
      text-align: center;
    }

    .expanded-events {
      position: absolute;
      top: 100%;
      left: 0;
      right: 0;
      background: var(--glass-bg);
      backdrop-filter: blur(20px);
      border: 1px solid var(--glass-border);
      border-radius: 12px;
      padding: 10px;
      margin-top: 5px;
      z-index: 100;
      box-shadow: var(--shadow);
      max-height: 200px;
      overflow-y: auto;
    }

    .expanded-event {
      padding: 6px 8px;
      margin-bottom: 4px;
      border-radius: 6px;
      font-size: 11px;
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .expanded-event.event-work,
    .expanded-event.event-personal,
    .expanded-event.event-other,
    .expanded-event.event-work *,
    .expanded-event.event-personal *,
    .expanded-event.event-other * {
      color: #000000 !important;
    }

    @keyframes modalEnter {
      from {
        opacity: 0;
        transform: scale(0.9) translateY(-10px);
      }
      to {
        opacity: 1;
        transform: scale(1) translateY(0);
      }
    }

    .modal-enter {
      animation: modalEnter 0.3s cubic-bezier(0.4, 0, 0.2, 1);
    }

    /* Responsive Design */
    @media (max-width: 768px) {
      .calendar-day {
        height: 100px;
        padding: 6px;
      }
      
      .day-number {
        font-size: 14px;
      }
      
      .event-item {
        font-size: 9px;
        padding: 2px 4px;
      }
    }

    /* Event items in modal */
    .event-item-modal {
      color: #000000 !important;
    }

    .event-item-modal * {
      color: #000000 !important;
    }
  </style>
</head>
<body>
  <!-- Theme Toggle -->
  <button class="theme-toggle" onclick="toggleTheme()">
    <i class="fas fa-moon" id="themeIcon"></i>
  </button>

  <div class="container mx-auto px-4 py-8">
    <!-- Header -->
    <div class="text-center mb-8">
      <h1 class="text-4xl font-bold mb-4">
        <i class="fas fa-microchip mr-3"></i>ESP32 Smart Calendar
      </h1>
      <p class="text-lg opacity-80">Adaptive Event Display & Intelligent Management</p>
    </div>

    <!-- Navigation -->
    <div class="glass rounded-xl p-6 mb-6">
      <div class="flex items-center justify-between">
        <button class="btn-secondary flex items-center gap-2" onclick="prevMonth()">
          <i class="fas fa-chevron-left"></i>
          Previous
        </button>
        
        <h2 class="text-2xl font-bold" id="monthYear"></h2>
        
        <button class="btn-secondary flex items-center gap-2" onclick="nextMonth()">
          Next
          <i class="fas fa-chevron-right"></i>
        </button>
      </div>
    </div>

    <!-- Calendar Grid -->
    <div class="glass rounded-xl p-6">
      <!-- Week Days Header -->
      <div class="grid grid-cols-7 gap-2 mb-4">
        <div class="text-center font-semibold py-2">Sun</div>
        <div class="text-center font-semibold py-2">Mon</div>
        <div class="text-center font-semibold py-2">Tue</div>
        <div class="text-center font-semibold py-2">Wed</div>
        <div class="text-center font-semibold py-2">Thu</div>
        <div class="text-center font-semibold py-2">Fri</div>
        <div class="text-center font-semibold py-2">Sat</div>
      </div>
      
      <!-- Calendar Days -->
      <div class="calendar-grid" id="calendar"></div>
    </div>
  </div>

  <!-- Event Modal -->
  <div id="modalOverlay" class="fixed inset-0 bg-black bg-opacity-50 z-50 hidden items-center justify-center backdrop-blur-sm">
    <div class="modal modal-enter p-6" id="eventModal">
      <div class="flex items-center justify-between mb-6">
        <h3 class="text-xl font-bold flex items-center gap-2">
          <i class="fas fa-calendar-day"></i>
          Events for <span id="modalDate" class="text-blue-400"></span>
        </h3>
        <button class="text-2xl opacity-60 hover:opacity-100 transition-opacity" onclick="closeModal()">
          <i class="fas fa-times"></i>
        </button>
      </div>

      <!-- Add Event Form -->
      <div class="mb-6 p-4 rounded-xl" style="background: var(--bg-secondary);">
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
          
          <button class="btn-primary flex items-center gap-2" onclick="addEvent()">
            <i class="fas fa-plus"></i>
            Add Event
          </button>
        </div>
      </div>

      <!-- Events List -->
      <div id="eventsList">
        <h4 class="font-semibold mb-4 flex items-center gap-2">
          <i class="fas fa-list"></i>
          Today's Events
        </h4>
        <div id="eventsContainer" class="space-y-2"></div>
      </div>
    </div>
  </div>

  <!-- Event Tooltip -->
  <div id="eventTooltip" class="event-tooltip"></div>

  <script>
    let currentDate = new Date();
    let events = {}; // Will be loaded from server
    let selectedDate = '';
    let expandedDay = null;
    let lastFetchTime = 0;
    const MAX_VISIBLE_EVENTS = 3;
    const FETCH_THROTTLE = 500; // Throttle API calls to 500ms

    // Load events from server when page loads
    document.addEventListener('DOMContentLoaded', function() {
      initTheme();
      loadEvents();
      renderCalendar();
    });

    // Load events from server with throttling
    function loadEvents() {
      const now = Date.now();
      if (now - lastFetchTime < FETCH_THROTTLE) {
        return Promise.resolve();
      }
      
      lastFetchTime = now;
      
      return fetch('/events')
        .then(response => {
          if (response.ok) {
            return response.json();
          } else {
            console.error('Failed to load events');
            return {};
          }
        })
        .then(data => {
          events = data;
          renderCalendar();
        })
        .catch(error => {
          console.error('Error loading events:', error);
          events = {};
        });
    }

    // Save events to server with throttling and retries
    function saveEvents() {
      const now = Date.now();
      if (now - lastFetchTime < FETCH_THROTTLE) {
        setTimeout(saveEvents, FETCH_THROTTLE);
        return;
      }
      
      lastFetchTime = now;
      
      const saveData = async (retries = 3) => {
        try {
          const response = await fetch('/events', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json'
            },
            body: JSON.stringify(events)
          });
          
          if (!response.ok) {
            console.error('Failed to save events');
            if (retries > 0) {
              setTimeout(() => saveData(retries - 1), 1000);
            }
          }
        } catch (error) {
          console.error('Error saving events:', error);
          if (retries > 0) {
            setTimeout(() => saveData(retries - 1), 1000);
          }
        }
      };
      
      saveData();
    }

    // Theme Management
    function initTheme() {
      const savedTheme = localStorage.getItem('calendarTheme') || 'dark';
      document.documentElement.setAttribute('data-theme', savedTheme);
      updateThemeIcon(savedTheme);
    }

    function toggleTheme() {
      const currentTheme = document.documentElement.getAttribute('data-theme');
      const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
      document.documentElement.setAttribute('data-theme', newTheme);
      localStorage.setItem('calendarTheme', newTheme);
      updateThemeIcon(newTheme);
    }

    function updateThemeIcon(theme) {
      const icon = document.getElementById('themeIcon');
      icon.className = theme === 'dark' ? 'fas fa-sun' : 'fas fa-moon';
    }

    // Calendar Rendering
    function renderCalendar() {
      const calendar = document.getElementById("calendar");
      const monthYear = document.getElementById("monthYear");
      calendar.innerHTML = '';

      const year = currentDate.getFullYear();
      const month = currentDate.getMonth();
      const firstDay = new Date(year, month, 1).getDay();
      const daysInMonth = new Date(year, month + 1, 0).getDate();

      monthYear.innerText = `${currentDate.toLocaleString('default', { month: 'long' })} ${year}`;

      // Empty cells for days before the month starts
      for (let i = 0; i < firstDay; i++) {
        calendar.innerHTML += `<div></div>`;
      }

      // Days of the month
      for (let day = 1; day <= daysInMonth; day++) {
        const dateKey = `${year}-${month + 1}-${day}`;
        const today = new Date();
        const isToday = today.getDate() === day && today.getMonth() === month && today.getFullYear() === year;

        const dayElement = createDayElement(day, dateKey, isToday);
        calendar.appendChild(dayElement);
      }
    }

    function createDayElement(day, dateKey, isToday) {
      const dayDiv = document.createElement('div');
      dayDiv.className = `calendar-day glass ${isToday ? 'today' : ''}`;
      dayDiv.onclick = function() { openModal(dateKey); };

      // Day number
      const dayNumber = document.createElement('div');
      dayNumber.className = 'day-number';
      dayNumber.textContent = day;
      dayDiv.appendChild(dayNumber);

      // Events container
      const eventsContainer = document.createElement('div');
      eventsContainer.className = 'events-container';
      
      const dayEvents = events[dateKey] || [];
      const sortedEvents = dayEvents.slice().sort((a, b) => a.time.localeCompare(b.time));
      
      // Show visible events
      const visibleEvents = sortedEvents.slice(0, MAX_VISIBLE_EVENTS);
      visibleEvents.forEach(function(event) {
        const eventElement = createEventElement(event);
        eventsContainer.appendChild(eventElement);
      });

      // Show "more" indicator if there are additional events
      if (sortedEvents.length > MAX_VISIBLE_EVENTS) {
        const moreElement = document.createElement('div');
        moreElement.className = 'show-more';
        moreElement.textContent = `+${sortedEvents.length - MAX_VISIBLE_EVENTS} more`;
        moreElement.onclick = function(e) {
          e.stopPropagation();
          toggleExpandedEvents(dayDiv, dateKey, sortedEvents);
        };
        eventsContainer.appendChild(moreElement);
      }

      dayDiv.appendChild(eventsContainer);
      return dayDiv;
    }

    function createEventElement(event) {
      const eventDiv = document.createElement('div');
      eventDiv.className = `event-item event-${event.category.toLowerCase()}`;
      
      const icon = getIconForCategory(event.category);
      eventDiv.innerHTML = `
        <span class="category-icon">${icon}</span>
        <span class="event-time">${event.time}</span>
        <span class="event-text">${truncateText(event.text, 15)}</span>
      `;

      // Add tooltip on hover
      eventDiv.onmouseenter = function(e) { showTooltip(e, event); };
      eventDiv.onmouseleave = hideTooltip;

      return eventDiv;
    }

    function getIconForCategory(category) {
      const icons = {
        'Work': '💼',
        'Personal': '🏠',
        'Other': '🎯'
      };
      return icons[category] || '📅';
    }

    function truncateText(text, maxLength) {
      return text.length > maxLength ? text.substring(0, maxLength) + '...' : text;
    }

    function toggleExpandedEvents(dayElement, dateKey, allEvents) {
      const existing = dayElement.querySelector('.expanded-events');
      
      if (existing) {
        existing.remove();
        expandedDay = null;
        return;
      }

      // Close any other expanded day
      if (expandedDay) {
        const oldExpanded = expandedDay.querySelector('.expanded-events');
        if (oldExpanded) oldExpanded.remove();
      }

      // Create expanded events container
      const expandedDiv = document.createElement('div');
      expandedDiv.className = 'expanded-events';

      allEvents.forEach(function(event) {
        const eventDiv = document.createElement('div');
        eventDiv.className = `expanded-event event-${event.category.toLowerCase()}`;
        eventDiv.innerHTML = `
          <span class="category-icon">${getIconForCategory(event.category)}</span>
          <span class="event-time">${event.time}</span>
          <span>${event.text}</span>
        `;
        expandedDiv.appendChild(eventDiv);
      });

      dayElement.style.position = 'relative';
      dayElement.appendChild(expandedDiv);
      expandedDay = dayElement;

      // Close when clicking outside
      setTimeout(function() {
        document.addEventListener('click', closeExpandedEvents, { once: true });
      }, 100);
    }

    function closeExpandedEvents(e) {
      if (expandedDay && !expandedDay.contains(e.target)) {
        const expanded = expandedDay.querySelector('.expanded-events');
        if (expanded) expanded.remove();
        expandedDay = null;
      }
    }

    // Tooltip Functions
    function showTooltip(e, event) {
      const tooltip = document.getElementById('eventTooltip');
      tooltip.innerHTML = `
        <div class="font-semibold">${event.time} - ${event.text}</div>
        <div class="text-sm opacity-75">${getIconForCategory(event.category)} ${event.category}</div>
      `;
      
      const rect = e.target.getBoundingClientRect();
      tooltip.style.left = rect.left + rect.width / 2 + 'px';
      tooltip.style.top = rect.top - 10 + 'px';
      tooltip.classList.add('show');
    }

    function hideTooltip() {
      const tooltip = document.getElementById('eventTooltip');
      tooltip.classList.remove('show');
    }

    // Modal Functions
    function openModal(dateKey) {
      selectedDate = dateKey;
      const formattedDate = formatDateForDisplay(dateKey);
      document.getElementById("modalDate").innerText = formattedDate;
      document.getElementById("eventText").value = '';
      document.getElementById("eventTime").value = '12:00';
      document.getElementById("eventCategory").value = 'Work';
      
      updateEventsList();
      document.getElementById("modalOverlay").classList.remove('hidden');
      document.getElementById("modalOverlay").classList.add('flex');
    }

    function closeModal() {
      document.getElementById("modalOverlay").classList.add('hidden');
      document.getElementById("modalOverlay").classList.remove('flex');
      
      // Close any expanded events
      if (expandedDay) {
        const expanded = expandedDay.querySelector('.expanded-events');
        if (expanded) expanded.remove();
        expandedDay = null;
      }
    }

    function formatDateForDisplay(dateKey) {
      const [year, month, day] = dateKey.split('-');
      const date = new Date(year, month - 1, day);
      return date.toLocaleDateString('en-US', {
        weekday: 'long',
        year: 'numeric',
        month: 'long',
        day: 'numeric'
      });
    }

    // Event Management
    function addEvent() {
      const text = document.getElementById("eventText").value.trim();
      const time = document.getElementById("eventTime").value;
      const category = document.getElementById("eventCategory").value;

      if (!text) {
        alert("Please enter event description.");
        return;
      }

      if (!events[selectedDate]) {
        events[selectedDate] = [];
      }

      events[selectedDate].push({ text, time, category });
      
      // Save to server
      saveEvents();
      
      document.getElementById("eventText").value = '';
      
      updateEventsList();
      renderCalendar();
    }

    function deleteEvent(index) {
      if (confirm("Are you sure you want to delete this event?")) {
        events[selectedDate].splice(index, 1);
        if (events[selectedDate].length === 0) {
          delete events[selectedDate];
        }
        
        // Save to server
        saveEvents();
        
        updateEventsList();
        renderCalendar();
      }
    }

    function updateEventsList() {
      const container = document.getElementById("eventsContainer");
      container.innerHTML = '';

      const dayEvents = events[selectedDate] || [];
      
      if (dayEvents.length === 0) {
        container.innerHTML = `
          <div class="text-center py-8 opacity-60">
            <i class="fas fa-calendar-times text-3xl mb-2"></i>
            <p>No events scheduled for this day</p>
          </div>
        `;
        return;
      }

      const sortedEvents = dayEvents.slice().sort((a, b) => a.time.localeCompare(b.time));
      
      sortedEvents.forEach(function(event, index) {
        const eventDiv = document.createElement('div');
        eventDiv.className = `flex items-center justify-between p-3 rounded-lg event-item-modal event-${event.category.toLowerCase()}`;
        eventDiv.innerHTML = `
          <div class="flex items-center gap-3">
            <span class="text-lg">${getIconForCategory(event.category)}</span>
            <div>
              <div class="font-semibold">${event.text}</div>
              <div class="text-sm opacity-75">${event.time} • ${event.category}</div>
            </div>
          </div>
          <button class="text-red-500 hover:text-red-700 transition-colors p-2" onclick="deleteEvent(${index})">
            <i class="fas fa-trash"></i>
          </button>
        `;
        container.appendChild(eventDiv);
      });
    }

    // Navigation Functions
    function prevMonth() {
      currentDate.setMonth(currentDate.getMonth() - 1);
      renderCalendar();
    }

    function nextMonth() {
      currentDate.setMonth(currentDate.getMonth() + 1);
      renderCalendar();
    }

    // Keyboard shortcuts
    document.addEventListener('keydown', function(e) {
      if (e.key === 'Escape') {
        closeModal();
      }
    });

    // Close modal when clicking overlay
    document.getElementById('modalOverlay').addEventListener('click', function(e) {
      if (e.target === e.currentTarget) {
        closeModal();
      }
    });
  </script>
</body>
</html>
)rawliteral");

  file.close();
  Serial.println("HTML file created successfully");
}

// Function to read events from SPIFFS with error checking
String readEventsFile() {
  if (!SPIFFS.exists(eventsFilePath)) {
    Serial.println("Events file not found");
    return "{}";
  }
  
  File file = SPIFFS.open(eventsFilePath, FILE_READ);
  if (!file) {
    Serial.println("Failed to open events file for reading");
    return "{}";
  }
  
  String content = file.readString();
  file.close();
  
  // Validate JSON
  DeserializationError error = deserializeJson(jsonDoc, content);
  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return "{}";
  }
  
  return content;
}

// Function to write events to SPIFFS with error checking
bool writeEventsFile(const String& content) {
  // Validate JSON before writing
  DeserializationError error = deserializeJson(jsonDoc, content);
  if (error) {
    Serial.print("Invalid JSON, not saving: ");
    Serial.println(error.c_str());
    return false;
  }

  File file = SPIFFS.open(eventsFilePath, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open events file for writing");
    return false;
  }
  
  size_t bytesWritten = file.print(content);
  file.close();
  
  if (bytesWritten == 0) {
    Serial.println("Events file write failed");
    return false;
  }
  
  Serial.println("Events file updated successfully");
  return true;
}

// Handler for root path
void handleRoot() {
  if (SPIFFS.exists(indexHtmlPath)) {
    File file = SPIFFS.open(indexHtmlPath, FILE_READ);
    if (!file) {
      server.send(500, "text/plain", "Failed to open HTML file");
      return;
    }
    
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "File not found");
  }
}

// Handler for getting events
void handleGetEvents() {
  String events = readEventsFile();
  server.send(200, "application/json", events);
}

// Handler for saving events
void handleSaveEvents() {
  if (server.hasArg("plain")) {
    String eventData = server.arg("plain");
    if (writeEventsFile(eventData)) {
      server.send(200, "text/plain", "Events saved");
    } else {
      server.send(400, "text/plain", "Failed to save events - invalid data");
    }
  } else {
    server.send(400, "text/plain", "No data received");
  }
}

// Memory management function to log memory usage
void logMemory() {
  Serial.print("Free heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.print(" bytes, Largest free block: ");
  Serial.print(ESP.getMaxAllocHeap());
  Serial.println(" bytes");
}

void setup() {
  // Start serial for debugging
  Serial.begin(115200);
  delay(1000); // Give serial monitor time to open
  
  Serial.println("\n\n--- ESP32 Calendar Server ---");
  Serial.println("Initializing...");
  
  // Initialize SPIFFS with error checking
  if (!initSPIFFS()) {
    Serial.println("CRITICAL ERROR: SPIFFS initialization failed!");
    Serial.println("Server cannot start without SPIFFS. Please check your ESP32 configuration.");
    while (1) {
      delay(1000); // Halt execution
    }
  }

  // Connect to Wi-Fi with feedback
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  // Add timeout and feedback for WiFi connection
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi connection FAILED!");
    Serial.println("Please check your WiFi credentials or router.");
    Serial.println("Continuing in offline mode...");
  } else {
    Serial.println("\nWiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  
  // Configure server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/events", HTTP_GET, handleGetEvents);
  server.on("/events", HTTP_POST, handleSaveEvents);
  
  // Handle CORS preflight requests
  server.enableCORS(true);
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  
  // Log initial memory state
  logMemory();
}

unsigned long lastWiFiCheck = 0;

void loop() {
  // Handle client requests
  server.handleClient();
  
  // Check WiFi status every 30 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - lastWiFiCheck >= 30000) {
    lastWiFiCheck = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost. Attempting to reconnect...");
      WiFi.reconnect();
    }
    
    // Periodically log memory status
    logMemory();
  }
  
  // Small delay to prevent watchdog timeouts
  delay(10);
}