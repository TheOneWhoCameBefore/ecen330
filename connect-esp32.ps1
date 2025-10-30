# The name of the device to look for in the usbipd list.
# Common names are "CP210x", "CH340", or "FTDI".
# Change this to match your device if needed.
$deviceName = "CH340"

Write-Host "Looking for device containing '$deviceName'..."

# Get the list of USB devices
$deviceList = usbipd list

# Find the line that contains our device name
$deviceLine = $deviceList | Where-Object { $_ -match $deviceName }

if ($null -eq $deviceLine) {
    Write-Host -ForegroundColor Red "Error: Device '$deviceName' not found. Make sure it's plugged in."
    pause
    exit
}

# Extract the BUSID from the line (it's the first word)
$busId = ($deviceLine.Split(' ') | Where-Object { $_ })[0]

Write-Host "Found device with BUSID: $busId"

# Bind the device
Write-Host "Binding device..."
usbipd bind --force --busid $busId

# Attach the device to WSL
Write-Host "Attaching device to WSL..."
usbipd attach --wsl --busid $busId

Write-Host -ForegroundColor Green "Successfully attached ESP32 to WSL!"
pause