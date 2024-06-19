// component/sideber.tsx
import React, { FC, ReactNode } from "react";
import { useSidebar } from "../provider/SidebarContext";
import { Normal } from "../presentation/normal";
import { Record } from "../presentation/record";
import { Mute } from "../presentation/mute";

interface SideBerProps {
  titleList: string[];
}

export const SideBer: FC<SideBerProps> = (props) => {
  const { titleList } = props;
  const { content, setContent } = useSidebar();

  const renderContent = (title: string) => {
    switch (title) {
      case "NORMAL":
        return <Normal />;
      case "RECORD":
        return <Record />;
      case "MUTE":
        return <Mute />;
      default:
        return null;
    }
  };

  return (
    <div
      style={{
        width: "100%",
        height: "100vh",
        backgroundColor: "#293033",
        display: "flex",
        flexDirection: "row",
        alignItems: "center",
        justifyContent: "center",
      }}
    >
      <div
        style={{
          width: "20%",
          height: "100%",
          backgroundColor: "#1e1f21",
          display: "flex",
          flexDirection: "column",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        {titleList.map((title, index) => (
          <div
            key={index}
            onClick={() => setContent(renderContent(title))}
            style={{
              width: "100%",
              height: "50px",
              display: "flex",
              flexDirection: "column",
              alignItems: "center",
              justifyContent: "center",
              color: "white",
              cursor: "pointer",
            }}
          >
            {title}
          </div>
        ))}
      </div>
      <div
        style={{
          width: "80%",
          height: "100%",
          backgroundColor: "#1e1f21",
          display: "flex",
          flexDirection: "column",
          alignItems: "center",
          justifyContent: "center",
        }}
      >
        {content}
      </div>
    </div>
  );
};
