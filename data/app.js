async function fetchData() {
    const response = await fetch('/read');
    return await response.json()        
}
async function updateUI(data) {
    let temp, hum, rssi, devicename;
    try {
        const data = await fetchData();

        let tempValue, tempUnit;
        if (data.display === 'c') {
            tempValue = data.celsius.temperature;
            tempUnit = 'C';
        } else {
            tempValue = data.fahrenheit.temperature;
            tempUnit = 'F';
        }
        temp = `${tempValue.toFixed(1)}°${tempUnit}`;
        hum = `${data.humidity.relative_perc.toFixed(1)}% RH`;
        rssi = `RSSI: ${data.wifi.rssi} dBm`;
        devicename = data.devicename;
    } catch (error) {
        console.error('Error fetching sensor data:', error);
        temp = hum = rssi = devicename = 'Error';
    }
    document.getElementById('devicename').textContent = devicename;
    document.getElementById('temperature').textContent = temp;
    document.getElementById('humidity').textContent = hum;
    document.getElementById('rssi').textContent = rssi;
    setTimeout(updateUI, 5000);
}

async function showNotification(message, type) {
    const notification = document.getElementById('notification');
    notification.innerHTML = message;
    notification.className = `notification ${type}`;
    notification.style.display = 'block';

    setTimeout(() => { notification.style.display = 'none'; }, 10000);
}

async function loadSettings() {
    try {
        const settings = await fetchData();

        document.getElementById('devicename').value = settings.devicename;
        document.getElementById('updateinterval').value = settings.updateinterval;
        document.getElementById('tempoffset').value = settings.celsius.offset;
        document.getElementById('humidityoffset').value = settings.humidity.relative_perc_offset;
        document.getElementById('showfahrenheit').checked = settings.display === 'f';
    } catch (error) {
        console.error('Error loading settings:', error);
    }
}

async function saveSettings(formData) {
    try {
        const response = await fetch('/settings', { method: 'POST', body: formData });
        const data = await response.json();

        if (data.status === "success") {
            showNotification("Settings saved successfully!", "success");
        } else if (data.status === "error" && data.errors.length > 0) {
            showNotification("Errors: <ul><li>" + data.errors.join("</li><li>") + "</li></ul>", "error");
        } else {
            showNotification("Unexpected response from server", "error");
        }
    } catch (error) {
        showNotification("Failed to save settings: " + error.message, "error");
    }
}