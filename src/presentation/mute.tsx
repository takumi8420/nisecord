import React, { FC, useState } from "react";

export const Mute: FC = () => {
  const [serverPort, setServerPort] = useState("");
  const [clientIpAddress, setServerIpAddress] = useState("");
  const [clientPort, setClientPort] = useState("");

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
      <p>Mute</p>
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
            onChange={(e) => setServerIpAddress(e.target.value)}
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
            }}
          >
            接続
          </button>
        </div>
      </div>
    </div>
  );
};
