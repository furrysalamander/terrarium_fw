const select = document.getElementById('ssidSelect');
const ssidInput = document.getElementById('ssid');
if (select) {
  select.addEventListener('change', () => {
    if (select.value) {
      ssidInput.value = select.value;
    }
  });
}
