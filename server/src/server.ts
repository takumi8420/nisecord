import express from "express";
import { spawn, ChildProcess } from "child_process";
import os from "os";
import cors from "cors";
import path from "path";
import fs from "fs";

const app = express();
const port = 8000;

app.use(cors()); // CORSを有効にする
app.use(express.json()); // JSONリクエストをパースするためのミドルウェア

const phoneProcesses: { [pid: number]: ChildProcess } = {}; // 実行中のプロセスを保持するマップ

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
  const port = req.query.port as string; // クエリパラメータから引数を取得し型キャスト

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

    const phoneProcess = spawn(commandPath, [port]);

    phoneProcess.stdout.on("data", (data) => {
      console.log(`stdout: ${data}`);
    });

    phoneProcess.stderr.on("data", (data) => {
      console.error(`stderr: ${data}`);
    });

    phoneProcess.on("close", (code) => {
      console.log(`child process exited with code ${code}`);
      delete phoneProcesses[phoneProcess.pid!]; // プロセス終了時にマップから削除
    });

    phoneProcesses[phoneProcess.pid!] = phoneProcess;

    res.send({
      ip_address: ip_address,
      port: port,
      pid: phoneProcess.pid,
      output: "Command executed successfully",
    });
  });
});

app.get("/connectPhone", (req, res) => {
  const ip_address = req.query.ipaddress as string; // クエリパラメータから引数を取得し型キャスト
  const port = req.query.port as string; // クエリパラメータから引数を取得し型キャスト

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

    const phoneProcess = spawn(commandPath, [ip_address, port]);

    phoneProcess.stdout.on("data", (data) => {
      console.log(`stdout: ${data}`);
    });

    phoneProcess.stderr.on("data", (data) => {
      console.error(`stderr: ${data}`);
    });

    phoneProcess.on("close", (code) => {
      console.log(`child process exited with code ${code}`);
      delete phoneProcesses[phoneProcess.pid!]; // プロセス終了時にマップから削除
    });

    phoneProcesses[phoneProcess.pid!] = phoneProcess;

    res.send({
      ip_address: ip_address,
      port: port,
      pid: phoneProcess.pid,
      output: "Command executed successfully",
    });
  });
});

app.post("/stopPhone", (req, res) => {
  const { pid } = req.body;
  if (pid && phoneProcesses[pid]) {
    try {
      process.kill(pid);
      delete phoneProcesses[pid]; // プロセスが停止したことを反映
      res.send({ message: "Phone process stopped successfully" });
    } catch (err: any) {
      res.status(500).send({
        error: "Failed to stop the phone process",
        details: err.message,
      });
    }
  } else {
    res
      .status(400)
      .send({ error: "No phone process ID provided or process not found" });
  }
});

app.post("/onMute", (req, res) => {
  const { pid } = req.body;
  if (pid && phoneProcesses[pid]) {
    const phoneProcess = phoneProcesses[pid];
    if (phoneProcess.stdin) {
      if (phoneProcess.stdin.writable) {
        phoneProcess.stdin.write("m");
        res.send({ message: "Mute command sent successfully" });
      } else {
        res.status(500).send({ error: "Phone process stdin is not writable" });
      }
    } else {
      res.status(500).send({ error: "Phone process stdin is not available" });
    }
  } else {
    res
      .status(400)
      .send({ error: "No phone process ID provided or process not found" });
  }
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
