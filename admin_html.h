const char adminHtml[] PROGMEM = R"AdminHTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Admin Portal - ESP32</title>
    <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@fortawesome/fontawesome-free@6.4.0/css/all.min.css">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
    <style>
        body {
            background: linear-gradient(135deg, #2c3e50 0%, #000000 100%);
            min-height: 100vh;
            color: white;
            font-family: 'Inter', sans-serif;
        }
        .glass {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(16px);
            border: 1px solid rgba(255, 255, 255, 0.1);
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.5);
            border-radius: 20px;
        }
        #customModalMessage {
            white-space: pre-wrap;
        }
        .btn {
            transition: all 0.2s;
        }
        .btn:hover { transform: translateY(-2px); }
    </style>
</head>
<body class="p-6">
    <div class="max-w-4xl mx-auto">
        <div class="flex justify-between items-center mb-8">
            <h1 class="text-3xl font-bold text-red-400"><i class="fas fa-user-shield"></i> Admin Portal</h1>
            <div>
                <a href="/" class="px-4 py-2 rounded bg-gray-600 hover:bg-gray-500 mr-2">Back to Calendar</a>
                <a href="/logout" class="px-4 py-2 rounded bg-red-600 hover:bg-red-500">Logout</a>
            </div>
        </div>

        <!-- Password Change Section -->
        <div class="glass p-6 mb-8">
            <h2 class="text-xl font-bold mb-4 flex items-center gap-2">
                <i class="fas fa-key"></i> Change Admin Password
            </h2>
            <form onsubmit="changePassword(event)" class="space-y-4 max-w-md">
                <input type="password" name="newPass" placeholder="New Password" class="w-full p-3 rounded bg-white bg-opacity-10 border border-gray-600 focus:border-red-400 outline-none text-white" required>
                <button type="submit" class="w-full py-3 rounded bg-red-600 hover:bg-red-500 font-bold btn">Update Password</button>
            </form>
            <div id="passMsg" class="mt-4 text-center font-bold"></div>
        </div>

        <!-- WiFi Settings Section -->
        <div class="glass p-6 mb-8 border-l-4 border-yellow-500">
            <h2 class="text-xl font-bold mb-4 flex items-center gap-2 text-yellow-400">
                <i class="fas fa-wifi"></i> WiFi Configuration
            </h2>
            <p class="text-gray-300 mb-4">
                Need to change the WiFi network? This will clear current settings and restart the device in Access Point mode.
            </p>
            <button onclick="resetWiFi()" class="w-full md:w-auto px-6 py-3 rounded bg-yellow-600 hover:bg-yellow-500 text-white font-bold shadow-lg transition btn flex items-center justify-center gap-2">
                <i class="fas fa-eraser"></i> Forget WiFi & Restart
            </button>
        </div>

        <!-- Access Control Section -->
        <div class="glass p-6 mb-8">
            <h2 class="text-xl font-bold mb-4 flex items-center gap-2">
                <i class="fas fa-eye"></i> Grant New Access
            </h2>
            <p class="text-sm text-gray-400 mb-4">Select a user and check the calendars they should be allowed to view.</p>
            
            <div class="space-y-4">
                 <div>
                    <label class="block mb-2 text-sm text-gray-300">Select User to Grant Access To:</label>
                    <select id="permUserSelect" onchange="renderPermUI()" class="w-full p-3 rounded bg-white bg-opacity-10 border border-gray-600 focus:border-blue-400 outline-none text-white">
                        <option value="" class="text-black">-- Select User --</option>
                    </select>
                </div>
                
                <div id="permEditor" class="hidden animate-fade-in">
                    <p class="mb-2 text-sm text-gray-300">Allow <span id="targetUserName" class="font-bold text-blue-400"></span> to view calendars of:</p>
                    <p class="mb-2 text-xs text-gray-400"><i class="fas fa-info-circle"></i> Check the users below and click Save.</p>
                    <div id="permCheckList" class="grid grid-cols-2 md:grid-cols-3 gap-2 p-4 bg-black bg-opacity-30 rounded border border-gray-700 max-h-60 overflow-y-auto">
                        <!-- Checkboxes -->
                    </div>
                    <button onclick="savePermissions()" class="mt-4 w-full py-3 rounded bg-blue-600 hover:bg-blue-500 font-bold btn">
                        <i class="fas fa-save mr-2"></i> Save Permissions
                    </button>
                </div>
            </div>
        </div>

        <!-- Permissions Overview Section -->
        <div class="glass p-6 mb-8 border-l-4 border-blue-500">
            <h2 class="text-xl font-bold mb-6 flex items-center gap-2 text-blue-400">
                <i class="fas fa-list-check"></i> Active Permissions Overview
            </h2>
            <div class="overflow-x-auto">
                <table class="w-full text-left">
                    <thead>
                        <tr class="border-b border-gray-700 text-gray-400 text-sm">
                            <th class="p-3">Viewer (Who sees)</th>
                            <th class="p-3">Target (Whose calendar)</th>
                            <th class="p-3 text-right">Action</th>
                        </tr>
                    </thead>
                    <tbody id="activePermsList">
                        <!-- Permissions will be loaded here -->
                    </tbody>
                </table>
                <div id="noPermsMsg" class="hidden text-center py-4 text-gray-500 italic">No custom permissions set.</div>
            </div>
        </div>

        <!-- User Management Section -->
        <div class="glass p-6">
            <h2 class="text-xl font-bold mb-6 flex items-center gap-2">
                <i class="fas fa-users-cog"></i> User Management
            </h2>
            <div class="overflow-x-auto">
                <table class="w-full text-left">
                    <thead>
                        <tr class="border-b border-gray-700">
                            <th class="p-3">Username</th>
                            <th class="p-3 text-right">Action</th>
                        </tr>
                    </thead>
                    <tbody id="userList">
                        <!-- Users will be loaded here -->
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    <!-- Custom Modal -->
    <div id="customModal" class="fixed inset-0 z-50 flex items-center justify-center bg-black bg-opacity-70 hidden opacity-0 transition-opacity duration-300">
        <div class="glass p-6 max-w-sm w-full mx-4 transform scale-95 transition-transform duration-300" id="customModalContent">
            <h3 id="customModalTitle" class="text-xl font-bold mb-4 text-white"></h3>
            <p id="customModalMessage" class="mb-6 text-gray-300 font-medium"></p>
            <div class="flex justify-end gap-3" id="customModalActions"></div>
        </div>
    </div>

    <script>
        let allUsers = [];
        let allPermissions = {};

        // Load data on start
        initData();

        async function initData() {
            await fetchUsers();
            await fetchPermissions();
        }

        async function fetchUsers() {
            try {
                const res = await fetch('/admin/users');
                const users = await res.json();
                allUsers = users;
                
                // Update User List Table
                const tbody = document.getElementById('userList');
                tbody.innerHTML = '';
                
                // Update Permission Dropdown
                const sel = document.getElementById('permUserSelect');
                // Keep selection if exists and valid
                const currentSel = sel.value;
                sel.innerHTML = '<option value="" class="text-black">-- Select User --</option>';

                users.forEach(user => {
                    // Table
                    if(user !== 'Admin') { 
                        const tr = document.createElement('tr');
                        tr.className = 'border-b border-gray-700 hover:bg-white hover:bg-opacity-5';
                        tr.innerHTML = `
                            <td class="p-3 font-mono text-lg">${user}</td>
                            <td class="p-3 text-right">
                                <button onclick="deleteUser('${user}')" class="px-3 py-1 bg-red-500 hover:bg-red-600 rounded text-sm btn text-white font-bold">
                                    <i class="fas fa-trash-alt"></i> Delete
                                </button>
                            </td>
                        `;
                        tbody.appendChild(tr);
                    }

                    const opt = document.createElement('option');
                    opt.value = user;
                    opt.innerText = user;
                    opt.className = "text-black";
                    sel.appendChild(opt);
                });
                
                if(currentSel && allUsers.includes(currentSel)) {
                    sel.value = currentSel;
                }
            } catch(e) {
                console.error('Failed to load users');
            }
        }

        async function fetchPermissions() {
            try {
                const res = await fetch('/admin/permissions');
                allPermissions = await res.json();
                renderActivePermsTable();
            } catch(e) { console.error('Failed perms'); }
        }

        function renderActivePermsTable() {
            const tbody = document.getElementById('activePermsList');
            const noMsg = document.getElementById('noPermsMsg');
            tbody.innerHTML = '';
            
            let hasPerms = false;
            
            for (const [viewer, targets] of Object.entries(allPermissions)) {
                targets.forEach(target => {
                    hasPerms = true;
                    const tr = document.createElement('tr');
                    tr.className = 'border-b border-gray-800 hover:bg-white hover:bg-opacity-5';
                    tr.innerHTML = `
                        <td class="p-3"><span class="px-2 py-1 rounded bg-blue-900 text-blue-200 text-xs font-bold">${viewer}</span></td>
                        <td class="p-3 text-gray-300">can see <span class="font-bold text-white">${target}</span></td>
                        <td class="p-3 text-right">
                            <button onclick="revokePermission('${viewer}', '${target}')" class="text-red-400 hover:text-red-200 text-sm transition">
                                <i class="fas fa-user-minus"></i> Revoke
                            </button>
                        </td>
                    `;
                    tbody.appendChild(tr);
                });
            }
            
            if (!hasPerms) {
                noMsg.classList.remove('hidden');
            } else {
                noMsg.classList.add('hidden');
            }
        }

        async function revokePermission(viewer, target) {
            showCustomModal('Revoke Access', `Revoke permission: ${viewer} will no longer be able to see ${target}'s calendar. Continue?`, true, async () => {
                if(allPermissions[viewer]) {
                    allPermissions[viewer] = allPermissions[viewer].filter(t => t !== target);
                    if(allPermissions[viewer].length === 0) delete allPermissions[viewer];
                }

                const formData = new FormData();
                formData.append('plain', JSON.stringify(allPermissions));

                try {
                    const res = await fetch('/admin/permissions', { method: 'POST', body: formData });
                    if(res.ok) {
                        renderActivePermsTable();
                        if(document.getElementById('permUserSelect').value === viewer) renderPermUI();
                    }
                } catch(e) { console.error(e); }
            });
        }

        function renderPermUI() {
            const user = document.getElementById('permUserSelect').value;
            const container = document.getElementById('permEditor');
            const list = document.getElementById('permCheckList');
            
            if(!user) {
                container.classList.add('hidden');
                return;
            }

            document.getElementById('targetUserName').innerText = user;
            container.classList.remove('hidden');
            list.innerHTML = '';

            const authorized = allPermissions[user] || [];

            allUsers.forEach(u => {
                if(u === user) return;

                const isChecked = authorized.includes(u) ? 'checked' : '';
                const div = document.createElement('div');
                div.className = 'flex items-center space-x-2 bg-white bg-opacity-5 p-2 rounded hover:bg-opacity-10 cursor-pointer transition';
                div.innerHTML = `
                    <input type="checkbox" id="chk_${u}" value="${u}" ${isChecked} class="form-checkbox h-5 w-5 text-blue-600 rounded focus:ring-blue-500 border-gray-300 bg-gray-700 border-none">
                    <label for="chk_${u}" class="cursor-pointer select-none flex-grow font-medium text-gray-200">${u}</label>
                `;
                div.onclick = (e) => {
                    if(e.target.tagName !== 'INPUT') {
                        const chk = div.querySelector('input');
                        chk.checked = !chk.checked;
                    }
                };
                list.appendChild(div);
            });
        }

        async function savePermissions() {
            const user = document.getElementById('permUserSelect').value;
            if(!user) return;

            const checks = document.querySelectorAll('#permCheckList input:checked');
            const grants = Array.from(checks).map(c => c.value);

            allPermissions[user] = grants;

            const formData = new FormData();
            formData.append('plain', JSON.stringify(allPermissions));

            try {
                const res = await fetch('/admin/permissions', { method: 'POST', body: formData });
                if(res.ok) {
                    showCustomModal('Success', 'Permissions saved successfully!', false);
                    renderActivePermsTable();
                } else {
                    showCustomModal('Error', 'Error saving permissions', false);
                }
            } catch(e) { showCustomModal('Error', 'Network error', false); }
        }

        function deleteUser(username) {
            showCustomModal('Delete User', `Are you sure you want to delete user "${username}"?`, true, async () => {
                const formData = new FormData();
                formData.append('username', username);
                
                const res = await fetch('/admin/delete_user', { method: 'POST', body: formData });
                if(res.ok) {
                    // Success, re-init
                    initData(); 
                } else {
                    showCustomModal('Error', 'Failed to delete user', false);
                }
            });
        }

        async function changePassword(e) {
            e.preventDefault();
            const form = e.target;
            const formData = new FormData(form);
            const msg = document.getElementById('passMsg');
            
            try {
                const res = await fetch('/admin/change_password', { method: 'POST', body: formData });
                if(res.ok) {
                    msg.textContent = "Password updated successfully!";
                    msg.className = "mt-4 text-center font-bold text-green-400";
                    form.reset();
                } else {
                    msg.textContent = "Error updating password.";
                    msg.className = "mt-4 text-center font-bold text-red-400";
                }
            } catch(e) {
                msg.textContent = "Connection failed.";
                msg.className = "mt-4 text-center font-bold text-red-400";
            }
        }

        function resetWiFi() {
            showCustomModal('WiFi Reset', '⚠️ WARNING: This will disconnect the device from current WiFi!\n\nThe device will restart and create an Access Point (ESP32-Calendar) so you can set up a new network.\n\nAre you sure?', true, () => {
                fetch('/admin/reset_wifi', { method: 'POST' })
                .then(res => {
                    if(res.ok) {
                        showCustomModal('WiFi Reset', 'WiFi settings cleared! Device is restarting...\n\nPlease connect to "ESP32-Calendar" hotspot in a moment.', false);
                        setTimeout(() => window.location.reload(), 3000);
                    } else {
                        showCustomModal('Error', 'Failed to reset WiFi.', false);
                    }
                })
                .catch(e => {
                    showCustomModal('WiFi Reset', 'Command sent. Device may be restarting already.', false);
                });
            });
        }

        // --- Custom Modal Logic ---
        function showCustomModal(title, message, isConfirm, onConfirm) {
            const modal = document.getElementById('customModal');
            const content = document.getElementById('customModalContent');
            
            document.getElementById('customModalTitle').innerText = title;
            document.getElementById('customModalTitle').className = `text-xl font-bold mb-4 ${title === 'Error' || title === 'Delete User' || title === 'WiFi Reset' ? 'text-red-400' : 'text-green-400'}`;
            document.getElementById('customModalMessage').innerHTML = message;
            
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
    </script>
</body>
</html>
)AdminHTML";