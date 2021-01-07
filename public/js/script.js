const authScreen = document.getElementById("auth");
const mainScreen = document.getElementById("main");

const loginInput = document.getElementById("loginInput");
const loginButton = document.getElementById("loginButton");

const startButton = document.getElementById("startTraffic");
const stopButton = document.getElementById("stopTraffic");

const aqiField = document.getElementById("aqi");

const red = document.getElementById("red");
const yellow = document.getElementById("yellow");
const green = document.getElementById("green");

const render = () => {
  const token = localStorage.getItem("token");
  if (token) {
    mainScreen.style.display = "grid";
    authScreen.style.display = "none";

    startButton.addEventListener("click", () => {
      yellow.classList.add("on");
      red.classList.remove("on");

      fetch("/api/v1/traffic/start", {
        method: "POST",
        headers: { token: token },
      }).then(() => {
        yellow.classList.remove("on");
        green.classList.add("on");
      });
    });

    stopButton.addEventListener("click", () => {
      yellow.classList.add("on");
      green.classList.remove("on");

      fetch("/api/v1/traffic/stop", {
        method: "POST",
        headers: { token: token },
      }).then(() => {
        yellow.classList.remove("on");
        red.classList.add("on");
      });
    });

    setInterval(() => {
      fetch("/api/v1/getAqiSensorValue", { headers: { token: token } })
        .then((response) => response.json())
        .then((data) => {
          aqiField.innerText = data.aqiSensor;
        });
    }, 2000);
  } else {
    mainScreen.style.display = "none";
    authScreen.style.display = "grid";
  }
};

const handleLogin = () => {
  const password = loginInput.value;
  console.log(password);
  if (password.length > 0) {
    fetch("/api/v1/login", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ password: password }),
    })
      .then((response) => response.json())
      .then((data) => {
        const { authToken } = data;
        localStorage.setItem("token", authToken);
        render();
      })
      .catch(() => {
        alert("API error");
      });
  }
};

loginButton.addEventListener("click", handleLogin);
render();
