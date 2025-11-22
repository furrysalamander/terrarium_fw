const sliderContainer = document.getElementById('brightnessSlider');
const sliderFill = document.getElementById('sliderFill');
const sliderHandle = document.getElementById('sliderHandle');
const numberInput = document.getElementById('brightnessInput');
const brightnessLabel = document.getElementById('brightnessValue');
const wifiStatusLabel = document.getElementById('wifiStatus');
const portalButton = document.getElementById('portalButton');
const portalStatusLabel = document.getElementById('portalStatus');
const portalInstructions = document.getElementById('portalInstructions');
const timezoneInput = document.getElementById('timezoneOffset');
const timezoneDisplay = document.getElementById('timezoneDisplay');
const localClock = document.getElementById('localClock');
const timezoneNote = document.getElementById('timezoneNote');
const portalDefaultMessage = portalInstructions ? portalInstructions.textContent : '';
const snapStops = [0, 25, 50, 75, 100];
const SNAP_THRESHOLD = 4;
let userAdjusting = false;
let brightnessDebounce;
let portalStateActive = false;
let timezoneSyncInFlight = false;
let timezoneMetadataSignature = null;
let currentBrightness = parseInt(brightnessLabel.textContent) || 0;

function clampBrightness(value) {
  let parsed = parseInt(value, 10);
  if (isNaN(parsed)) parsed = 0;
  return Math.max(0, Math.min(100, parsed));
}

function maybeSnap(value) {
  for (const stop of snapStops) {
    if (Math.abs(value - stop) <= SNAP_THRESHOLD) {
      return stop;
    }
  }
  return value;
}

function updateSliderPosition(value) {
  if (!sliderContainer || !sliderFill || !sliderHandle) return;
  const clamped = Math.max(0, Math.min(100, value));
  const percentage = clamped + '%';
  sliderFill.style.width = percentage;
  sliderHandle.style.left = percentage;
}

function updateBrightnessControls(value) {
  currentBrightness = value;
  numberInput.value = value;
  brightnessLabel.innerText = value + '%';
  updateSliderPosition(value);
}

function postBrightness(value) {
  clearTimeout(brightnessDebounce);
  const body = new URLSearchParams();
  body.append('value', value);
  brightnessDebounce = setTimeout(() => {
    fetch('/brightness', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: body.toString()
    }).catch(() => {});
  }, 160);
}

function handleBrightnessInput(rawValue, snap) {
  let value = clampBrightness(rawValue);
  if (snap) {
    value = maybeSnap(value);
  }
  updateBrightnessControls(value);
  postBrightness(value);
  return value;
}

function getSliderValueFromEvent(event) {
  if (!sliderContainer) return 0;
  const rect = sliderContainer.getBoundingClientRect();
  const x = event.clientX - rect.left;
  const percentage = Math.max(0, Math.min(100, (x / rect.width) * 100));
  return Math.round(percentage);
}

let isDragging = false;

if (sliderContainer) {
  sliderContainer.addEventListener('mousedown', event => {
    isDragging = true;
    userAdjusting = true;
    const value = getSliderValueFromEvent(event);
    handleBrightnessInput(value, true);
    event.preventDefault();
  });

  document.addEventListener('mousemove', event => {
    if (!isDragging) return;
    const value = getSliderValueFromEvent(event);
    handleBrightnessInput(value, true);
    event.preventDefault();
  });

  document.addEventListener('mouseup', () => {
    if (isDragging) {
      isDragging = false;
      userAdjusting = false;
    }
  });

  sliderContainer.addEventListener('touchstart', event => {
    isDragging = true;
    userAdjusting = true;
    const touch = event.touches[0];
    const value = getSliderValueFromEvent(touch);
    handleBrightnessInput(value, true);
    event.preventDefault();
  });

  document.addEventListener('touchmove', event => {
    if (!isDragging) return;
    const touch = event.touches[0];
    const value = getSliderValueFromEvent(touch);
    handleBrightnessInput(value, true);
    event.preventDefault();
  });

  document.addEventListener('touchend', () => {
    if (isDragging) {
      isDragging = false;
      userAdjusting = false;
    }
  });
}

numberInput.addEventListener('input', event => {
  userAdjusting = true;
  handleBrightnessInput(event.target.value, false);
});
numberInput.addEventListener('change', event => {
  handleBrightnessInput(event.target.value, false);
  userAdjusting = false;
});
numberInput.addEventListener('blur', () => { userAdjusting = false; });

updateSliderPosition(currentBrightness);

function setPortalUi(state) {
  if (!portalButton || !portalStatusLabel || !portalInstructions) {
    return;
  }
  portalStateActive = !!state.active;
  portalStatusLabel.innerText = portalStateActive ? 'Active' : 'Idle';
  portalButton.disabled = portalStateActive;
  portalButton.innerText = portalStateActive ? 'Portal Running' : 'Launch Wi-Fi Setup';
  if (state.message) {
    portalInstructions.innerText = state.message;
  } else {
    portalInstructions.innerText = portalStateActive ? `Join "${state.ssid}" (pass: ${state.password}).` : portalDefaultMessage;
  }
}

if (portalButton) {
  portalButton.addEventListener('click', () => {
    portalButton.disabled = true;
    portalButton.innerText = 'Starting...';
    fetch('/wifi/portal', { method: 'POST' })
      .then(resp => resp.json())
      .then(data => setPortalUi(data))
      .catch(() => {
        setPortalUi({ active: false, message: 'Unable to start portal. Please retry.' });
      });
  });
}

function daysInMonth(month, year) {
  return new Date(year, month, 0).getDate();
}

function describeTransition(date) {
  const day = date.getDate();
  const week = Math.floor((day - 1) / 7) + 1;
  const isLast = day + 7 > daysInMonth(date.getMonth() + 1, date.getFullYear());
  return {
    month: date.getMonth() + 1,
    week: isLast ? 5 : week,
    weekday: date.getDay(),
    hour: date.getHours(),
    minute: date.getMinutes()
  };
}

function refineTransitionWindow(lowMs, highMs) {
  let low = lowMs;
  let high = highMs;
  let lowOffset = new Date(low).getTimezoneOffset();
  while (high - low > 60000) {
    const mid = Math.floor((low + high) / 2);
    const midOffset = new Date(mid).getTimezoneOffset();
    if (midOffset === lowOffset) {
      low = mid;
    } else {
      high = mid;
    }
  }
  return {
    utcMillis: high,
    beforeOffset: -new Date(low).getTimezoneOffset(),
    afterOffset: -new Date(high).getTimezoneOffset(),
    localDate: new Date(high)
  };
}

function detectTimezoneTransitions(year) {
  const transitions = [];
  const start = Date.UTC(year - 1, 0, 1);
  const end = Date.UTC(year + 2, 0, 1);
  let prevOffset = new Date(start).getTimezoneOffset();
  for (let ts = start; ts < end; ts += 3600000) {
    const offset = new Date(ts).getTimezoneOffset();
    if (offset !== prevOffset) {
      transitions.push(refineTransitionWindow(ts - 3600000, ts));
      prevOffset = offset;
      if (transitions.length >= 4) {
        break;
      }
    }
  }
  return transitions;
}

function buildTimezoneAbbr(name) {
  const parts = (name || '').split(/[\/_]/);
  const segment = parts[parts.length - 1] || name || 'LOC';
  const letters = segment.replace(/[^A-Za-z]/g, '').toUpperCase();
  const padded = (letters + 'LOC').slice(0, 3);
  return padded || 'LOC';
}

function formatPosixOffset(minutes) {
  const inverted = -minutes;
  const sign = inverted < 0 ? '-' : '';
  const absMinutes = Math.abs(inverted);
  const hours = Math.floor(absMinutes / 60);
  const mins = absMinutes % 60;
  return mins === 0 ? `${sign}${hours}` : `${sign}${hours}:${mins.toString().padStart(2, '0')}`;
}

function formatPosixRule(rule) {
  let str = `M${rule.month}.${rule.week}.${rule.weekday}`;
  if (rule.hour !== 2 || rule.minute !== 0) {
    str += `/${rule.hour}`;
    if (rule.minute) {
      str += `:${rule.minute.toString().padStart(2, '0')}`;
    }
  }
  return str;
}

function formatOffsetLabel(minutes) {
  const sign = minutes >= 0 ? '+' : '-';
  const abs = Math.abs(minutes);
  const hours = Math.floor(abs / 60)
    .toString()
    .padStart(2, '0');
  const mins = (abs % 60)
    .toString()
    .padStart(2, '0');
  return `UTC${sign}${hours}:${mins}`;
}

function analyzeTimezoneMetadata() {
  if (typeof Intl === 'undefined' || typeof Date === 'undefined') {
    return null;
  }
  const tzName = Intl.DateTimeFormat().resolvedOptions().timeZone || 'UTC';
  const now = new Date();
  const currentOffset = -now.getTimezoneOffset();
  const year = now.getFullYear();
  const offsets = [];
  for (let month = 0; month < 12; month++) {
    const sample = new Date(year, month, 1, 12, 0, 0);
    offsets.push(-sample.getTimezoneOffset());
  }
  const minOffset = Math.min(...offsets);
  const maxOffset = Math.max(...offsets);
  const hasDst = minOffset !== maxOffset;
  if (!hasDst) {
    return {
      tzName,
      currentOffset,
      standardOffset: currentOffset,
      daylightOffset: currentOffset,
      hasDst: false,
      posix: ''
    };
  }
  const transitions = detectTimezoneTransitions(year);
  const start = transitions.find(t => t.afterOffset > t.beforeOffset);
  const end = transitions.find(t => t.afterOffset < t.beforeOffset);
  if (!start || !end) {
    return {
      tzName,
      currentOffset,
      standardOffset: currentOffset,
      daylightOffset: currentOffset,
      hasDst: false,
      posix: ''
    };
  }
  const standardOffset = Math.min(start.beforeOffset, start.afterOffset, end.beforeOffset, end.afterOffset);
  const daylightOffset = Math.max(start.beforeOffset, start.afterOffset, end.beforeOffset, end.afterOffset);
  const startRule = describeTransition(start.localDate);
  const endRule = describeTransition(end.localDate);
  const stdAbbr = buildTimezoneAbbr(tzName);
  const dstAbbr = `${stdAbbr.slice(0, 2)}D`;
  const posix = `${stdAbbr}${formatPosixOffset(standardOffset)}${dstAbbr}${formatPosixOffset(daylightOffset)},${formatPosixRule(startRule)},${formatPosixRule(endRule)}`;
  return {
    tzName,
    currentOffset,
    standardOffset,
    daylightOffset,
    hasDst: true,
    posix
  };
}

function syncTimezoneIfNeeded(deviceOffset) {
  if (timezoneSyncInFlight) {
    return;
  }
  const meta = analyzeTimezoneMetadata();
  if (!meta) {
    return;
  }
  if (timezoneDisplay) {
    const label = `${meta.tzName} (${formatOffsetLabel(meta.currentOffset)})`;
    timezoneDisplay.value = `Auto ${label}`;
  }
  const signature = JSON.stringify({
    tz: meta.tzName,
    std: meta.standardOffset,
    dst: meta.daylightOffset,
    posix: meta.posix
  });
  if (signature === timezoneMetadataSignature && meta.currentOffset === deviceOffset) {
    return;
  }
  timezoneSyncInFlight = true;
  const body = new URLSearchParams();
  body.append('offset', meta.currentOffset);
  body.append('stdOffset', meta.standardOffset);
  body.append('dstOffset', meta.daylightOffset);
  body.append('hasDst', meta.hasDst ? '1' : '0');
  body.append('tzName', meta.tzName);
  if (meta.posix) {
    body.append('posix', meta.posix);
  }
  fetch('/timezone', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: body.toString()
  })
    .then(response => {
      if (!response.ok) {
        throw new Error('Timezone sync failed');
      }
      timezoneMetadataSignature = signature;
    })
    .catch(() => {
      timezoneMetadataSignature = null;
    })
    .finally(() => {
      timezoneSyncInFlight = false;
    });
}

function refreshStatus() {
  fetch('/status')
    .then(resp => resp.json())
    .then(data => {
      if (!userAdjusting) {
        updateBrightnessControls(data.brightness);
      }
      document.getElementById('lightStatus').innerText = data.lightOn ? 'ON (' + data.appliedBrightness + '%)' : 'OFF';
      if (data.schedule.enabled) {
        document.getElementById('scheduleStatus').innerText = data.schedule.active ? 'Active now' : 'Idle until next window';
      } else {
        document.getElementById('scheduleStatus').innerText = 'Disabled';
      }
      document.getElementById('clockStatus').innerText = data.clockSynced ? 'Yes' : 'No (fallback mode)';
      if (timezoneInput && typeof data.timezoneOffset === 'number') {
        timezoneInput.value = data.timezoneOffset;
      }
      if (timezoneDisplay && data.timezoneLabel) {
        timezoneDisplay.value = `Auto ${data.timezoneLabel}`;
      }
      if (typeof data.timezoneOffset === 'number') {
        syncTimezoneIfNeeded(data.timezoneOffset);
      }
      if (localClock && data.localTime) {
        localClock.innerText = data.localTime;
      }
      if (wifiStatusLabel) {
        wifiStatusLabel.innerText = data.wifi.connected ? `Connected (${data.wifi.ssid || 'hidden'})` : 'Disconnected';
      }
      if (data.portal) {
        setPortalUi({
          active: data.portal.active,
          message: data.portal.active ? `Join "${data.portal.ssid}" (pass: ${data.portal.password}).` : portalDefaultMessage,
          ssid: data.portal.ssid,
          password: data.portal.password
        });
      }
      const ts = new Date();
      document.getElementById('lastUpdate').innerText = ts.toLocaleTimeString();
    })
    .catch(() => {});
}

setInterval(refreshStatus, 5000);
window.addEventListener('load', refreshStatus);
