const express = require('express');
const path = require('path');
const app = express();
const port = 3000;

// Global state
let state = {
    buttonState: "enabled",
    relayState: "off",
    timerState: "disabled",
    timerDuration: "30"
};

// Serve static files from current directory
app.use(express.static('.'));

// State endpoint
app.get('/state', (req, res) => {
    res.json(state);
});

// Control endpoints
app.get('/enable-button', (req, res) => {
    state.buttonState = 'enabled';
    redirectToSource(res, req.query.source);
});

app.get('/disable-button', (req, res) => {
    state.buttonState = 'disabled';
    redirectToSource(res, req.query.source);
});

app.get('/enable-timer', (req, res) => {
    state.timerState = 'enabled';
    redirectToSource(res, req.query.source);
});

app.get('/disable-timer', (req, res) => {
    state.timerState = 'disabled';
    redirectToSource(res, req.query.source);
});

app.get('/force-on', (req, res) => {
    state.relayState = 'on';
    redirectToSource(res, req.query.source);
});

app.get('/force-off', (req, res) => {
    state.relayState = 'off';
    redirectToSource(res, req.query.source);
});

app.get('/update-timer-duration', (req, res) => {
    if (req.query.durationSeconds) {
        state.timerDuration = req.query.durationSeconds;
    }
    redirectToSource(res, req.query.source);
});

function redirectToSource(res, source = '/') {
    res.send(`<script>window.location.href = "${source}";</script>`);
}

// Start server
app.listen(port, () => {
    console.log(`Test server running at http://localhost:${port}`);
    console.log('Press Ctrl+C to stop');
}); 