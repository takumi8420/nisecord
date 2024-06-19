import express from "express";
import { exec } from "child_process";
import os from "os";
import cors from "cors";
import path from "path";
import fs from "fs";

const app = express();
const port = 8000;

app.use(cors()); // CORSを有効にする

app.get("/api/hello", (req, res) => {
  res.send({ message: "Hello from the server!" });
});

app.get("/listenPhone", (req, res) => {
  const getIpAddress = (): string | null => {
    const interfaces = os.networkInterfaces();
    for (const iface of Object.values(interfaces)) {
      if (!iface) continue; // ifaceがundefinedでないことを確認
      for (const alias of iface) {
        if (alias.family === "IPv4" && !alias.internal) {
          return alias.address;
        }
      }
    }
    return null;
  };

  const ip_address = getIpAddress();
  const port = req.query.port; // クエリパラメータから引数を取得

  if (!ip_address || !port) {
    res.status(400).send("IPアドレスとポート番号を指定してください。");
    return;
  }

  const commandPath = path.join(__dirname, "bin", "phone");
  console.log(`Command path: ${commandPath}`);

  fs.access(commandPath, fs.constants.X_OK, (err) => {
    if (err) {
      console.error(`File access error: ${err.message}`);
      res.status(500).send({ error: `File access error: ${err.message}` });
      return;
    }

    const command = `${commandPath} ${port}`;
    console.log(`Executing command: ${command}`);

    const _ = exec(command, (error, stdout, stderr) => {
      if (error) {
        console.error(`exec error: ${error.message}`);
        res.status(500).send({ error: error.message, stderr: stderr });
        return;
      }
      // エラーが発生しない場合に2秒後にレスポンスを返す
      setTimeout(() => {
        res.send({
          ip_address: ip_address,
          port: port,
          output: "Command executed successfully",
        });
      }, 2000);
    });
  });
});

app.get("/connectPhone", (req, res) => {
  const ip_address = req.query.ipaddress;
  const port = req.query.port;

  if (!ip_address || !port) {
    res.status(400).send("IPアドレスとポート番号を指定してください。");
    return;
  }

  const commandPath = path.join(__dirname, "bin", "phone");
  console.log(`Command path: ${commandPath}`);

  fs.access(commandPath, fs.constants.X_OK, (err) => {
    if (err) {
      console.error(`File access error: ${err.message}`);
      res.status(500).send({ error: `File access error: ${err.message}` });
      return;
    }

    const command = `${commandPath} ${ip_address} ${port}`;
    console.log(`Executing command: ${command}`);

    const _ = exec(command, (error, stdout, stderr) => {
      if (error) {
        console.error(`exec error: ${error.message}`);
        res.status(500).send({ error: error.message, stderr: stderr });
        return;
      }
      // エラーが発生しない場合に2秒後にレスポンスを返す
      setTimeout(() => {
        res.send({
          ip_address: ip_address,
          port: port,
          output: "Command executed successfully",
        });
      }, 2000);
    });
  });
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
