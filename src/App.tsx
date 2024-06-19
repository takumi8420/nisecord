import "./App.css";
import { SideBer } from "./component/sideber";

import { SidebarProvider } from "./provider/SidebarContext";

function App() {
  return (
    <SidebarProvider>
      <div className="App">
        <header className="App-header">
          <SideBer titleList={["NORMAL", "RECORD", "MUTE"]}></SideBer>
        </header>
      </div>
    </SidebarProvider>
  );
}

export default App;
