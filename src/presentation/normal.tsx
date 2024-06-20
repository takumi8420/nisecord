import React, { FC, useState } from "react";

export const Normal: FC = () => {
  const [serverPort, setServerPort] = useState("");
  const [clientIpAddress, setClientIpAddress] = useState("");
  const [clientPort, setClientPort] = useState("");
  const [pid, setPid] = useState<string>("");

  // port番号を引数にしてサーバーを立ち上げる
  const createServer = async () => {
    console.log("サーバーを作成");
    try {
      const response = await fetch(
        `http://localhost:8000/listenPhone?port=${serverPort}`
      );
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      const data = await response.json();
      console.log("IP Address:", data.ip_address);
      console.log("Port:", data.port);
      console.log("Output:", data.output);
      console.log("PID:", data.pid);
      //ipaddressをセット
      setClientIpAddress(data.ip_address);
      // pidをセット
      setPid(data.pid);
    } catch (error) {
      console.error("Failed to fetch:", error);
    }
  };

  const connectServer = async () => {
    console.log("サーバーに接続");
    try {
      const response = await fetch(
        `http://localhost:8000/connectPhone?ipaddress=${clientIpAddress}&port=${clientPort}`
      );
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      const data = await response.json();
      console.log("IP Address:", data.ip_address);
      console.log("Port:", data.port);
      console.log("Output:", data.output);
      console.log("PID:", data.pid);
      // pidをセット
      setPid(data.pid);
    } catch (error) {
      console.error("Failed to fetch:", error);
    }
  };

  const stopServer = async () => {
    console.log("サーバーを停止");
    try {
      const response = await fetch("http://localhost:8000/stopPhone", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ pid: parseInt(pid, 10) }),
      });
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      const data = await response.json();
      console.log("Message:", data.message);
    } catch (error) {
      console.error("Failed to fetch:", error);
    }
  };

  const muteServer = async () => {
    console.log("サーバーをミュート");
    try {
      const response = await fetch("http://localhost:8000/onMute", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({ pid: parseInt(pid, 10) }),
      });
      if (!response.ok) {
        throw new Error("Network response was not ok");
      }
      const data = await response.json();
      console.log("Message:", data.message);
    } catch (error) {
      console.error("Failed to fetch:", error);
    }
  };

  return (
    <div
      style={{
        width: "100%",
        height: "100vh",
        backgroundColor: "#778f96",
        display: "flex",
        flexDirection: "column",
        alignItems: "center",
        justifyContent: "center",
      }}
    >
      <p>Normal</p>

      <div>
        <p>IP Address: {clientIpAddress}</p>
        <p>Port: {clientPort}</p>
        {/* <p>PID: {pid}</p> */}
      </div>
      <div
        style={{
          width: "100%",
          height: "100vh",
          backgroundColor: "#778f96",
          display: "flex",
          flexDirection: "row",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        <div
          style={{
            width: "50%",
            height: "50%",
            backgroundColor: "#778f96",
            display: "flex",
            flexDirection: "column",
            alignItems: "center",
            justifyContent: "center",
          }}
        >
          <p>サーバー</p>
          <input
            type="text"
            placeholder="Port"
            value={serverPort}
            onChange={(e) => setServerPort(e.target.value)}
            style={{
              marginBottom: "10px",
              padding: "5px",
              borderRadius: "5px",
              border: "1px solid #ccc",
            }}
          />
          <button
            onClick={createServer}
            style={{
              width: "100px",
              height: "50px",
              backgroundColor: "#1e1f21",
              color: "white",
              border: "none",
              borderRadius: "5px",
              marginBottom: "10px",
            }}
          >
            立ち上げ
          </button>
          <button
            onClick={stopServer}
            style={{
              width: "100px",
              height: "50px",
              backgroundColor: "#1e1f21",
              color: "white",
              border: "none",
              borderRadius: "5px",
              marginBottom: "10px",
            }}
          >
            終了
          </button>
        </div>
        <div
          style={{
            width: "50%",
            height: "50%",
            display: "flex",
            flexDirection: "column",
            alignItems: "center",
            justifyContent: "center",
          }}
        >
          <p>クライアント</p>
          <input
            type="text"
            placeholder="IP Address"
            value={clientIpAddress}
            onChange={(e) => setClientIpAddress(e.target.value)}
            style={{
              marginBottom: "10px",
              padding: "5px",
              borderRadius: "5px",
              border: "1px solid #ccc",
            }}
          />
          <input
            type="text"
            placeholder="Port"
            value={clientPort}
            onChange={(e) => setClientPort(e.target.value)}
            style={{
              marginBottom: "10px",
              padding: "5px",
              borderRadius: "5px",
              border: "1px solid #ccc",
            }}
          />

          <button
            onClick={connectServer}
            style={{
              width: "100px",
              height: "50px",
              backgroundColor: "#1e1f21",
              color: "white",
              border: "none",
              borderRadius: "5px",
              marginBottom: "10px",
            }}
          >
            接続
          </button>
          <button
            onClick={stopServer}
            style={{
              width: "100px",
              height: "50px",
              backgroundColor: "#1e1f21",
              color: "white",
              border: "none",
              borderRadius: "5px",
              marginBottom: "10px",
            }}
          >
            終了
          </button>
          <button
            onClick={muteServer}
            style={{
              width: "100px",
              height: "50px",
              backgroundColor: "#1e1f21",
              color: "white",
              border: "none",
              borderRadius: "5px",
              marginBottom: "10px",
            }}
          >
            ミュート
          </button>
        </div>
      </div>
    </div>
  );
};
