const path = require('path');
const electron = require('electron');
const app = electron.app;
const BrowserWindow = electron.BrowserWindow;
const nativeImage = electron.nativeImage;

const ipc = electron.ipcMain;
const Menu = electron.Menu;
const Tray = electron.Tray;

let mainWindow;
let appIcon;
let logoPath = path.join(__dirname + "/assets", 'logo.png');
let occupiedImage = nativeImage.createFromPath(path.join(__dirname + "/assets", 'occupied.png'));
let availableImage = nativeImage.createFromPath(path.join(__dirname + "/assets", 'available.png'));

ipc.on('put-in-tray', function (event) {
  appIcon = new Tray(logoPath);
  const contextMenu = Menu.buildFromTemplate([
    {
      label: 'Hide window',
      click: function () {
        mainWindow.hide();
      }
    },
    {
      label: 'Quit',
      click: function () {
        event.sender.send('tray-removed')
        appIcon.destroy()
        app.quit();
      }
    }]);
  appIcon.setContextMenu(contextMenu);
  appIcon.on('click', function () {
    mainWindow.show();
  });
})

ipc.on('occupied', function (event) {
  appIcon.setImage(occupiedImage);
})

ipc.on('available', function (event) {
  appIcon.setImage(availableImage);
})

app.on('window-all-closed', function () {
  if (appIcon) appIcon.destroy();
})

function createWindow() {
  mainWindow = new BrowserWindow({ 
    width: 600, 
    height: 800,
    icon: logoPath});
  mainWindow.setMenu(null);
  // mainWindow.webContents.openDevTools();
  mainWindow.loadURL(`file://${__dirname}/index.html`)
  mainWindow.on('closed', function () {
    mainWindow = null;
  });
}

app.on('ready', createWindow);

app.on('window-all-closed', function () {
  // On OS X it is common for applications and their menu bar
  // to stay active until the user quits explicitly with Cmd + Q
  if (process.platform !== 'darwin') {
    app.quit();
  }
})

app.on('activate', function () {
  // On OS X it's common to re-create a window in the app when the
  // dock icon is clicked and there are no other windows open.
  if (mainWindow === null) {
    createWindow();
  }
})

app.on('minimize', function () {
  app.hide();
});
