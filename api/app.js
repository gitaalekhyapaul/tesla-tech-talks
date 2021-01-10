/*Importing Required Packages*/
const express = require("express");
const bodyParser = require("body-parser");
const dotenv = require("dotenv");
const axios = require("axios");
const path = require("path");

/*Initialising the Express Application*/
const app = express();
dotenv.config();

/*Adding the neccesary middlewares*/
app.use(bodyParser.json());

/* A very basic and insecure route protection middleware */
const routeProtectionMiddleware = (req, res, next) => {
  if (!req.headers["token"]) {
    return res.status(403).json({
      success: false,
      message: "Not Authorised!",
    });
  }
  const authToken = Buffer.from(
    req.headers["token"].toString(),
    "base64"
  ).toString();
  if (authToken.indexOf("gitaalekhyapaul::") !== -1) {
    next();
  } else {
    res.status(403).json({
      success: false,
      message: "Not Authorised!",
    });
  }
};

/* Function to clear set all lights to OFF */
const resetLights = async () => {
  await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
    ledType: "RED",
    setLed: "OFF",
  });
  await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
    ledType: "YELLOW",
    setLed: "OFF",
  });
  await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
    ledType: "GREEN",
    setLed: "OFF",
  });
};

/* API routes */
app.post("/api/v1/login", (req, res, next) => {
  const password = req.body["password"];
  console.log(password);
  if (password) {
    const authToken = Buffer.from(`gitaalekhyapaul::${password}`).toString(
      "base64"
    );
    res.status(200).json({
      success: true,
      authToken,
    });
  } else {
    res.status(400).json({
      success: false,
      message: "password cannot be null.",
    });
  }
});

app.post(
  "/api/v1/traffic/start",
  routeProtectionMiddleware,
  async (req, res, next) => {
    await resetLights();
    await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
      ledType: "YELLOW",
      setLed: "ON",
    });

    /* A cool way to introduce delays in execution (thread-blocking) */
    await new Promise((resolve, reject) => setTimeout(resolve, 2000));

    await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
      ledType: "YELLOW",
      setLed: "OFF",
    });
    await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
      ledType: "GREEN",
      setLed: "ON",
    });
    res.json({
      success: true,
      traffic: "started",
    });
  }
);

app.post(
  "/api/v1/traffic/stop",
  routeProtectionMiddleware,
  async (req, res, next) => {
    await resetLights();
    await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
      ledType: "YELLOW",
      setLed: "ON",
    });

    /* A cool way to introduce delays in execution (thread-blocking) */
    await new Promise((resolve, reject) => setTimeout(resolve, 2000));

    await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
      ledType: "YELLOW",
      setLed: "OFF",
    });
    await axios.post(`${process.env.NODEMCU_HOSTNAME}/api/v1/changeLed`, {
      ledType: "RED",
      setLed: "ON",
    });
    res.json({
      success: true,
      traffic: "stopped",
    });
  }
);

app.get(
  "/api/v1/getAqiSensorValue",
  routeProtectionMiddleware,
  async (req, res, next) => {
    const { data } = await axios.get(
      `${process.env.NODEMCU_HOSTNAME}/api/v1/getSensorValue`
    );
    res.json({
      success: true,
      aqiSensor: data.sensorValue,
    });
  }
);

/* Serving the web application statically in production environment */
if (process.env.NODE_ENV === "production") {
  app.use(express.static(path.join(__dirname, "..", "public")));
  app.use("*", (req, res, next) =>
    res.sendFile(path.join(__dirname, "..", "public", "index.html"))
  );
}

/*Adding a 404 Handler*/
app.use((req, res, next) => {
  res.status(404).json({
    success: false,
    message: `Cannot ${req.method} ${req.url}`,
  });
});

/* Server starts listening on the specified port*/
Promise.all([resetLights()])
  .then(() => {
    app.listen(process.env.PORT, () => {
      console.log(
        `Server:${process.env.NODE_ENV} Listening on Port ${process.env.PORT}`
      );
    });
  })
  .catch((_) => {
    console.error("Error in starting server!");
    process.exit(1);
  });
