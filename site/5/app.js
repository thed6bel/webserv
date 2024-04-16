const express = require('express');
const fs = require('fs');
const path = require('path');
const app = express();

const DIRECTORY = path.join(__dirname, '/fileUploaded');

app.get('/files', (req, res) => {
    fs.readdir(DIRECTORY, (err, files) => {
        if (err) {
            console.error("Failed to list files:", err);
            res.status(500).send("Failed to list files.");
            return;
        }
        res.json(files);
    });
});

app.listen(3000, () => console.log('App listening on port 3000!'));