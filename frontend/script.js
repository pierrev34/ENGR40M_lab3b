document.addEventListener('DOMContentLoaded', () => {
    const mazeEl = document.getElementById('maze');
    const statusEl = document.getElementById('status');
    let maze = [];
    let playerPos = {x: 1, y: 1};
    let endPos = {x: 6, y: 6};
    let serialPort = null;
    let reader = null;
    let writer = null;
    let arduinoConnected = false;
    
    // Arduino Connection
    async function connectToArduino() {
        try {
            if ('serial' in navigator) {
                serialPort = await navigator.serial.requestPort();
                await serialPort.open({ baudRate: 9600 });
                
                reader = serialPort.readable.getReader();
                writer = serialPort.writable.getWriter();
                
                statusEl.textContent = 'Connected to Arduino - sending maze data...';
                arduinoConnected = true;
                
                // Send a test message to Arduino
                console.log('Sending test message to Arduino');
                await writer.write(new TextEncoder().encode("TEST:CONNECTED\n"));
                
                // Force send the maze data immediately
                await sendMazeToArduino();
                
                // Start reading from Arduino
                readFromArduino();
            } else {
                statusEl.textContent = 'Web Serial API not supported by your browser';
                // Fall back to keyboard controls
            }
        } catch (error) {
            console.error('Error connecting to Arduino:', error);
            statusEl.textContent = 'Failed to connect to Arduino - using keyboard controls';
        }
    }
    
    // Read data from Arduino
    async function readFromArduino() {
        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    break;
                }
                
                // Process incoming data from Arduino
                const command = new TextDecoder().decode(value);
                console.log('Received from Arduino:', command);
                
                // Show data in status
                statusEl.textContent = 'Arduino: ' + command.trim();
                
                // Process command
                handleArduinoCommand(command.trim());
            }
        } catch (error) {
            console.error('Error reading from Arduino:', error);
            statusEl.textContent = 'Connection lost - reconnecting...';
            arduinoConnected = false;
        } finally {
            if (reader) {
                reader.releaseLock();
            }
        }
    }
    
    // Send maze data to Arduino
    async function sendMazeToArduino() {
        if (!arduinoConnected || !writer) return;
        
        try {
            // Send player position
            await writer.write(new TextEncoder().encode(`P:${playerPos.x},${playerPos.y}\n`));
            
            // Send exit position
            await writer.write(new TextEncoder().encode(`E:${endPos.x},${endPos.y}\n`));
            
            // Send maze data (row by row)
            for (let y = 0; y < 8; y++) {
                const rowData = maze[y].join('');
                await writer.write(new TextEncoder().encode(`M:${y}:${rowData}\n`));
            }
        } catch (error) {
            console.error('Error sending to Arduino:', error);
            statusEl.textContent = 'Error communicating with Arduino';
        }
    }
    
    // Process commands from Arduino
    function handleArduinoCommand(command) {
        // Log all commands from Arduino
        console.log('Arduino command:', command);
        
        // Process ready messages
        if (command.startsWith('READY:')) {
            statusEl.textContent = 'Arduino connected - LED matrix active';
            // Send a test move command
            if (writer) {
                writer.write(new TextEncoder().encode("MOVE:RIGHT\n"));
            }
        }
        
        // Process position updates
        else if (command.startsWith('POS:')) {
            const pos = command.substring(4).split(',');
            if (pos.length === 2) {
                const x = parseInt(pos[0]);
                const y = parseInt(pos[1]);
                
                // Update player position in web UI
                playerPos = {x, y};
                renderMaze();
            }
        }
        
        // Process win messages
        else if (command.startsWith('WIN:')) {
            createMaze(); // Generate new maze
        }
    }
    
    // Initialize maze using Prim's algorithm
    function createMaze() {
        // Initialize with all walls
        maze = Array(8).fill().map(() => Array(8).fill(1));
        
        // Remove top-left corner walls
        maze[0][0] = 0;
        maze[0][1] = 0;
        maze[1][0] = 0;
        
        // Set start position
        const startX = 1, startY = 1;
        maze[startY][startX] = 0;
        playerPos = {x: startX, y: startY};
        
        // Implement Prim's algorithm
        const walls = [];
        const addWallsToList = (x, y) => {
            [[0,1], [1,0], [0,-1], [-1,0]].forEach(([dx, dy]) => {
                const nx = x + dx*2;
                const ny = y + dy*2;
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                    walls.push({x: nx, y: ny, wx: x+dx, wy: y+dy});
                }
            });
        };
        
        addWallsToList(startX, startY);
        
        // While we have walls to process
        while (walls.length > 0) {
            // Pick a random wall
            const wallIndex = Math.floor(Math.random() * walls.length);
            const {x, y, wx, wy} = walls[wallIndex];
            walls.splice(wallIndex, 1);
            
            // If the cell on the opposite side isn't visited yet
            if (maze[y][x] === 1) {
                // Mark the cell and the wall as path
                maze[y][x] = 0;
                maze[wy][wx] = 0;
                
                // Add new walls
                addWallsToList(x, y);
            }
        }
        
        // Set end position (furthest from start)
        endPos = findFurthestPoint(startX, startY);
        
        // Ensure the maze has open spots near start and end
        maze[playerPos.y][playerPos.x] = 0; // Start cell
        maze[endPos.y][endPos.x] = 0; // End cell
        
        // Render and send to Arduino
        renderMaze();
        sendMazeToArduino();
    }
    
    // BFS to find the point furthest from start
    function findFurthestPoint(startX, startY) {
        const dirs = [[0, 1], [1, 0], [0, -1], [-1, 0]];
        const visited = Array(8).fill().map(() => Array(8).fill(false));
        const queue = [{x: startX, y: startY, dist: 0}];
        let furthest = {x: startX, y: startY, dist: 0};
        
        visited[startY][startX] = true;
        
        while (queue.length > 0) {
            const {x, y, dist} = queue.shift();
            
            // If this is the furthest point so far
            if (dist > furthest.dist && maze[y][x] === 0) {
                furthest = {x, y, dist};
            }
            
            // Check all four directions
            for (const [dx, dy] of dirs) {
                const nx = x + dx;
                const ny = y + dy;
                
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 && 
                    !visited[ny][nx] && maze[ny][nx] === 0) {
                    visited[ny][nx] = true;
                    queue.push({x: nx, y: ny, dist: dist + 1});
                }
            }
        }
        
        // Make sure the end point is at least some distance away
        if (furthest.dist < 5) {
            // If no good point found, just set a fixed position
            return {x: 6, y: 6};
        }
        
        return {x: furthest.x, y: furthest.y};
    }
    
    // Move player and check win condition
    function movePlayer(newX, newY) {
        // Check if move is valid
        if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8 && maze[newY][newX] !== 1) {
            playerPos = {x: newX, y: newY};
            renderMaze();
            
            // Send updated position to Arduino
            if (arduinoConnected) {
                writer.write(new TextEncoder().encode(`P:${playerPos.x},${playerPos.y}\n`));
            }
            
            // Check win condition
            if (playerPos.x === endPos.x && playerPos.y === endPos.y) {
                setTimeout(() => {
                    createMaze();
                }, 300);
            }
        }
    }
    
    // Render the maze on screen
    function renderMaze() {
        mazeEl.innerHTML = '';
        
        for (let y = 0; y < 8; y++) {
            for (let x = 0; x < 8; x++) {
                const cell = document.createElement('div');
                cell.className = 'cell';
                
                if (maze[y][x] === 1) {
                    cell.classList.add('wall');
                } else if (x === playerPos.x && y === playerPos.y) {
                    cell.classList.add('player');
                } else if (x === endPos.x && y === endPos.y) {
                    cell.classList.add('end');
                }
                
                mazeEl.appendChild(cell);
            }
        }
    }
    
    // Handle keyboard controls
    document.addEventListener('keydown', (e) => {
        let newX = playerPos.x;
        let newY = playerPos.y;
        let direction = '';
        
        switch(e.key) {
            case 'ArrowUp':
                newY--;
                direction = 'UP';
                break;
            case 'ArrowDown':
                newY++;
                direction = 'DOWN';
                break;
            case 'ArrowLeft':
                newX--;
                direction = 'LEFT';
                break;
            case 'ArrowRight':
                newX++;
                direction = 'RIGHT';
                break;
            // WASD keys for testing
            case 'w':
            case 'W':
                newY--;
                direction = 'UP';
                break;
            case 's':
            case 'S':
                newY++;
                direction = 'DOWN';
                break;
            case 'a':
            case 'A':
                newX--;
                direction = 'LEFT';
                break;
            case 'd':
            case 'D':
                newX++;
                direction = 'RIGHT';
                break;
            default: return;
        }
        
        // Move player in web UI
        movePlayer(newX, newY);
        
        // Try to send to Arduino if connected
        if (arduinoConnected && writer) {
            try {
                writer.write(new TextEncoder().encode(`MOVE:${direction}\n`));
                console.log(`Sent to Arduino: MOVE:${direction}`);
            } catch (error) {
                console.error('Error sending to Arduino:', error);
                statusEl.textContent = 'Failed to send to Arduino - using keyboard only';
                arduinoConnected = false;
            }
        }
    });
    
    // New maze button
    document.getElementById('newMaze').addEventListener('click', createMaze);
    
    // Connect Arduino button
    document.getElementById('connectArduino').addEventListener('click', connectToArduino);
    
    // Initialize the game without Arduino connection first
    createMaze();
});