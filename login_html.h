const char loginHtml[] PROGMEM = R"LoginHTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Login - ESP32 Calendar</title>
    <link href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@fortawesome/fontawesome-free@6.4.0/css/all.min.css">
    <style>
        body {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            font-family: 'Inter', sans-serif;
        }
        .glass {
            background: rgba(255, 255, 255, 0.15);
            backdrop-filter: blur(16px);
            border: 1px solid rgba(255, 255, 255, 0.2);
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
            border-radius: 20px;
            padding: 2rem;
            width: 90%;
            max-width: 400px;
            color: white;
            position: relative;
        }
        .input-group {
            margin-bottom: 1.5rem;
        }
        .input-group label {
            display: block;
            margin-bottom: 0.5rem;
            font-size: 0.9rem;
            opacity: 0.9;
        }
        .form-input {
            width: 100%;
            padding: 10px;
            border-radius: 10px;
            border: 1px solid rgba(255,255,255,0.3);
            background: rgba(255,255,255,0.1);
            color: white;
            outline: none;
            transition: all 0.3s;
        }
        .form-input:focus {
            background: rgba(255,255,255,0.2);
            border-color: white;
        }
        .btn {
            width: 100%;
            padding: 12px;
            border: none;
            border-radius: 10px;
            font-weight: bold;
            cursor: pointer;
            transition: transform 0.2s;
        }
        .btn-primary {
            background: white;
            color: #764ba2;
        }
        .btn-secondary {
            background: transparent;
            border: 1px solid rgba(255,255,255,0.5);
            color: white;
            margin-top: 10px;
        }
        .btn:hover {
            transform: scale(1.02);
        }
        .hidden { display: none; }
        .msg-box { 
            margin-bottom: 1rem; 
            text-align: center; 
            font-size: 0.9rem; 
            padding: 10px; 
            border-radius: 8px;
            display: none;
        }
        .error { background: rgba(255, 107, 107, 0.2); border: 1px solid rgba(255, 107, 107, 0.5); color: #ffcccc; }
        .success { background: rgba(50, 255, 126, 0.2); border: 1px solid rgba(50, 255, 126, 0.5); color: #ccffdd; }
    </style>
</head>
<body>
    <div class="glass">
        <h2 class="text-2xl font-bold text-center mb-6" id="title">Login</h2>
        
        <div id="messageBox" class="msg-box"></div>
        
        <!-- Login Form -->
        <form id="loginForm" onsubmit="handleAuth(event, '/login')">
            <div class="input-group">
                <label>Username</label>
                <input type="text" name="username" class="form-input" required>
            </div>
            <div class="input-group">
                <label>Password</label>
                <input type="password" name="password" class="form-input" required>
            </div>
            <button type="submit" class="btn btn-primary">Sign In</button>
            <button type="button" class="btn btn-secondary" onclick="toggleMode()">Create Account</button>
        </form>

        <!-- Register Form -->
        <form id="registerForm" class="hidden" onsubmit="handleAuth(event, '/register')">
            <div class="input-group">
                <label>Choose Username</label>
                <input type="text" name="username" class="form-input" required>
            </div>
            <div class="input-group">
                <label>Choose Password</label>
                <input type="password" name="password" class="form-input" required>
            </div>
            <button type="submit" class="btn btn-primary">Register</button>
            <button type="button" class="btn btn-secondary" onclick="toggleMode()">Back to Login</button>
        </form>
    </div>

    <script>
        function showMessage(text, isError = true) {
            const box = document.getElementById('messageBox');
            box.textContent = text;
            box.className = 'msg-box ' + (isError ? 'error' : 'success');
            box.style.display = 'block';
        }

        function toggleMode() {
            document.getElementById('loginForm').classList.toggle('hidden');
            document.getElementById('registerForm').classList.toggle('hidden');
            const isLogin = !document.getElementById('loginForm').classList.contains('hidden');
            document.getElementById('title').textContent = isLogin ? 'Login' : 'Create Account';
            document.getElementById('messageBox').style.display = 'none';
        }

        async function handleAuth(e, endpoint) {
            e.preventDefault();
            const formData = new FormData(e.target);
            const msgBox = document.getElementById('messageBox');
            msgBox.style.display = 'none';
            
            try {
                const response = await fetch(endpoint, {
                    method: 'POST',
                    body: formData
                });
                
                if (response.ok) {
                    if (endpoint === '/register') {
                        showMessage('Registration successful! Redirecting to login...', false);
                        setTimeout(() => {
                            toggleMode();
                            showMessage('Please log in with your new account.', false);
                        }, 1500);
                    } else {
                        window.location.href = '/';
                    }
                } else {
                    const text = await response.text();
                    showMessage(text || 'Authentication failed', true);
                }
            } catch (err) {
                showMessage('Network connection error', true);
            }
        }
    </script>
</body>
</html>
)LoginHTML";