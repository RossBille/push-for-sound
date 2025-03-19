# Push for Sound Test Server

A lightweight Node.js server for testing the Push for Sound interface without the ESP32 hardware.

## Setup

1. Make sure you have [Node.js](https://nodejs.org/) installed
2. Install dependencies:
   ```bash
   npm install
   ```

## Running the Server

Start the server with:
```bash
npm start
```

The server will run at `http://localhost:3000`. You should see:
```
Test server running at http://localhost:3000
Press Ctrl+C to stop
```

## Available Endpoints

The server simulates all the ESP32 endpoints:

- `GET /state` - Returns current system state
- `GET /enable-button` - Enables the button
- `GET /disable-button` - Disables the button
- `GET /enable-timer` - Enables the timer
- `GET /disable-timer` - Disables the timer
- `GET /force-on` - Forces relays on
- `GET /force-off` - Forces relays off
- `GET /update-timer-duration` - Updates timer duration

## State Structure

The server maintains a state object with the following structure:
```javascript
{
    buttonState: "enabled" | "disabled",
    relayState: "on" | "off",
    timerState: "enabled" | "disabled",
    timerDuration: string // number of seconds
}
```

## Development

The server consists of two main files:
- `test_server.js` - The Express server implementation
- `index.html` - The web interface

The server automatically serves static files from the current directory, so any changes to `index.html` will be immediately available on refresh.

## Notes

- State is maintained in memory and resets when the server restarts
- All endpoints support the `source` query parameter for redirect after action
- The server uses Express.js for simplicity and reliability 