
(function () {
    const ipc = require('electron').ipcRenderer
    var config = {
        apiKey: "AIzaSyCPUg984F6pX3-GrsJ-6_Ap9oqRRweN0Bc",
        authDomain: "mentortoilet.firebaseapp.com",
        databaseURL: "https://mentortoilet.firebaseio.com",
        storageBucket: "mentortoilet.appspot.com",
    },
        devicesRaw;

    firebase.initializeApp(config);

    ipc.send('put-in-tray');
    firebase.database().ref().child('devices').once('value', function (data) {
        devicesRaw = data.val();
        var devices = flatten(devicesRaw),
            tableBody = document.getElementById('table-body');
        devices.sort(compareByCode);
        for (i = 0; i < devices.length; i++) {
            var device = devices[i];
            buildDeviceDOMElement(tableBody, device);
            bindValueChangingEvent(device);
            if (device.key === getPrefferedToilet()) {
                buildPrefferedToilet(device);
            }
        }
    });

    function flatten(devices) {
        var keys = Object.keys(devices),
            i = keys.length,
            result = [],
            device;
        while (i--) {
            device = devices[keys[i]];
            device.key = keys[i];
            result.unshift(device);
        }
        return result;
    };

    function compareByCode(deviceA, deviceB) {
        if (deviceA.code < deviceB.code) return -1;
        if (deviceA.code > deviceB.code) return 1;
        return 0;
    }

    function buildDeviceDOMElement(tableBody, device) {
        var informationHolder = document.createElement('tr'),
            location = document.createElement('td'),
            type = document.createElement('td'),
            isAvailable = document.createElement('td');

        informationHolder.setAttribute('id', device.key);
        informationHolder.appendChild(location);
        informationHolder.appendChild(type);
        informationHolder.appendChild(isAvailable);
        tableBody.appendChild(informationHolder);

        informationHolder.addEventListener('click', function (event) {
            setPrefferedToilet(device.key);
            if (device.key === getPrefferedToilet()) {
                buildPrefferedToilet(devicesRaw[device.key]);
            }
        });
    }

    function getPrefferedToilet() {
        return localStorage.getItem("prefferedToilet");
    }

    function setPrefferedToilet(key) {
        localStorage.setItem("prefferedToilet", key);
    }

    function buildPrefferedToilet(device) {
        var prefferedHolder = document.getElementById('preffered'),
            prefferedInfo = document.createElement('p');
        removeAllChildNodes(prefferedHolder);
        prefferedInfo.innerHTML = 'Location: ' + device.location + '<br />Type: ' + device.type + '<br />Is Available: ' + !device.occupied;
        prefferedHolder.setAttribute('class', 'jumbotron text-center ' + (device.occupied ? 'occupied' : 'available'));
        prefferedHolder.appendChild(prefferedInfo);
        if (device.occupied) {
            ipc.send('occupied');
        } else {
            ipc.send('available');
        }
        var deviceHolder = document.getElementById(device.key);
    }

    function removeAllChildNodes(parent) {
        while (parent.firstChild) {
            parent.removeChild(parent.firstChild);
        }
    }

    function bindValueChangingEvent(device) {
        firebase.database().ref().child('devices').child(device.key).on('value', function (data) {
            devicesRaw[device.key] = data.val();
            var informationHolder = document.getElementById(device.key);
            informationHolder.childNodes[0].textContent = data.val().location;
            informationHolder.childNodes[1].textContent = data.val().type;
            informationHolder.childNodes[2].textContent = !data.val().occupied;
            informationHolder.setAttribute('class', data.val().occupied ? "danger" : "success");

            if (device.key === getPrefferedToilet()) {
                buildPrefferedToilet(data.val());
            }
        });
    }

})();
