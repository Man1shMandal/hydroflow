#include <WiFi.h>
#include <WebServer.h>

// ============================================================
// WIFI
// ============================================================
// Home Wi-Fi lives in wifi_secrets.h (git-ignored). Copy wifi_secrets.example.h to start.
#if __has_include("wifi_secrets.h")
#include "wifi_secrets.h"
#endif
#ifndef HOME_WIFI_SSID
#define HOME_WIFI_SSID ""
#define HOME_WIFI_PASS ""
#endif
const char* ssid     = HOME_WIFI_SSID;
const char* password = HOME_WIFI_PASS;

// ============================================================
// RELAY / PUMP CONFIGURATION
// Pump 1 = GPIO 14
// Pump 2 = GPIO 27
// Pump 3 = GPIO 26
// Pump 4 = GPIO 25
// ============================================================
const uint8_t relayPins[4] = {14, 27, 26, 25};

// Most common 4-channel relay boards are ACTIVE LOW.
// If yours works backwards, swap these two values.
const uint8_t RELAY_ON  = LOW;
const uint8_t RELAY_OFF = HIGH;

bool pumpState[4] = {false, false, false, false};

WebServer server(80);

// ============================================================
// WEB INTERFACE
// ============================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">

<title>Pump Control System</title>

<style>

*{
    box-sizing:border-box;
}

html,body{
    margin:0;
    padding:0;
    background:#070b0e;
    color:#d7e0e5;
    font-family:Arial,Helvetica,sans-serif;
}

body{
    min-height:100vh;
}

/* ==========================================================
   TOP HEADER
   ========================================================== */

.topbar{
    height:72px;
    background:
      linear-gradient(180deg,#182027,#0d1216);
    border-bottom:2px solid #34424c;

    display:flex;
    align-items:center;
    justify-content:space-between;

    padding:0 28px;

    box-shadow:0 4px 18px #000;
}

.brand{
    display:flex;
    align-items:center;
    gap:15px;
}

.logo{
    width:42px;
    height:42px;

    border:2px solid #42a5c6;
    border-radius:5px;

    position:relative;

    background:#0a1419;

    box-shadow:
      inset 0 0 15px rgba(66,165,198,.15),
      0 0 12px rgba(66,165,198,.15);
}

.logo:before{
    content:"";
    position:absolute;

    left:8px;
    right:8px;
    top:19px;

    height:3px;
    background:#42a5c6;
}

.logo:after{
    content:"";
    position:absolute;

    width:12px;
    height:12px;

    border:3px solid #42a5c6;
    border-radius:50%;

    left:12px;
    top:10px;
}

.title{
    font-size:18px;
    font-weight:700;
    letter-spacing:2px;
}

.subtitle{
    font-size:10px;
    color:#71818c;
    letter-spacing:1.6px;
    margin-top:5px;
}

.systemStatus{
    display:flex;
    align-items:center;
    gap:10px;

    padding:9px 14px;

    background:#0b1312;

    border:1px solid #264139;
    border-radius:3px;

    font-family:monospace;
    font-size:12px;
}

.onlineLamp{
    width:9px;
    height:9px;

    background:#28e282;
    border-radius:50%;

    box-shadow:
      0 0 6px #28e282,
      0 0 14px rgba(40,226,130,.7);
}


/* ==========================================================
   PANEL
   ========================================================== */

.panel{
    max-width:1300px;
    margin:25px auto;
    padding:0 18px 40px;
}

.panelHeader{
    display:flex;
    justify-content:space-between;
    align-items:flex-end;

    margin-bottom:13px;
}

.panelTitle{
    font-size:13px;
    font-weight:bold;
    letter-spacing:2px;
    color:#a9b6bd;
}

panelSmall{}

.panelSmall{
    font-family:monospace;
    font-size:10px;
    color:#596871;
}


/* ==========================================================
   PROCESS AREA
   ========================================================== */

.processPanel{
    position:relative;

    border:1px solid #29353c;

    background-color:#0a0f12;

    background-image:
      linear-gradient(rgba(65,87,98,.08) 1px,transparent 1px),
      linear-gradient(90deg,rgba(65,87,98,.08) 1px,transparent 1px);

    background-size:25px 25px;

    padding:28px 24px 25px;

    box-shadow:
      inset 0 0 50px rgba(0,0,0,.8),
      0 8px 25px rgba(0,0,0,.5);
}


/* CORNER BOLTS */

.bolt{
    position:absolute;

    width:7px;
    height:7px;

    border-radius:50%;

    background:#56636b;

    box-shadow:
      inset 1px 1px 1px #8a959b,
      inset -1px -1px 1px #20272b;
}

.b1{left:8px;top:8px;}
.b2{right:8px;top:8px;}
.b3{left:8px;bottom:8px;}
.b4{right:8px;bottom:8px;}


/* ==========================================================
   SCHEMATIC HEADER
   ========================================================== */

.diagramTitle{
    font-family:monospace;
    font-size:11px;

    color:#60717a;

    margin-bottom:30px;

    letter-spacing:1px;
}

.pipeHeader{
    position:relative;

    height:45px;

    margin:0 6% 10px;
}

.mainPipe{
    position:absolute;

    top:18px;
    left:0;
    right:0;

    height:5px;

    background:#41515a;

    box-shadow:
      0 1px 0 #75838a,
      0 -1px 0 #1a2429;
}

.mainPipe:before{
    content:"SUCTION MANIFOLD";

    position:absolute;

    top:-22px;
    left:0;

    font-family:monospace;
    font-size:9px;

    color:#62727b;
}

.flowArrow{
    position:absolute;

    right:-3px;
    top:12px;

    width:0;
    height:0;

    border-top:8px solid transparent;
    border-bottom:8px solid transparent;
    border-left:13px solid #60747e;
}


/* ==========================================================
   PUMP GRID
   ========================================================== */

.pumps{
    display:grid;

    grid-template-columns:
      repeat(4,minmax(190px,1fr));

    gap:18px;
}


/* ==========================================================
   INDIVIDUAL PUMP MODULE
   ========================================================== */

.pumpModule{
    position:relative;

    min-height:430px;

    background:
      linear-gradient(180deg,
        rgba(23,31,36,.92),
        rgba(10,15,18,.96));

    border:1px solid #303d44;

    padding:14px;

    transition:
      border .25s,
      box-shadow .25s;
}

.pumpModule.running{
    border-color:#267b57;

    box-shadow:
      inset 0 0 25px rgba(38,123,87,.08),
      0 0 15px rgba(40,226,130,.06);
}

.moduleTop{
    display:flex;
    align-items:center;
    justify-content:space-between;

    padding-bottom:10px;

    border-bottom:1px solid #29343a;
}

.equipmentID{
    font-family:monospace;
    font-size:14px;
    font-weight:bold;

    color:#d5dde1;
}

.channel{
    font-family:monospace;
    font-size:9px;

    color:#667780;
}


/* ==========================================================
   STATUS LAMP
   ========================================================== */

.statusArea{
    display:flex;
    align-items:center;
    gap:8px;

    margin-top:12px;
}

.statusLamp{
    width:10px;
    height:10px;

    border-radius:50%;

    background:#6d2424;

    box-shadow:
      inset 0 0 2px #000,
      0 0 5px rgba(255,40,40,.2);

    transition:.25s;
}

.running .statusLamp{
    background:#2bea87;

    box-shadow:
      0 0 6px #2bea87,
      0 0 14px rgba(43,234,135,.55);
}

.statusText{
    font-family:monospace;

    font-size:10px;
    letter-spacing:1px;

    color:#ad6565;
}

.running .statusText{
    color:#42e89a;
}


/* ==========================================================
   PUMP SCHEMATIC
   ========================================================== */

.pumpDrawing{
    height:185px;

    display:flex;
    align-items:center;
    justify-content:center;

    position:relative;
}

.pumpSvg{
    width:175px;
    height:145px;

    overflow:visible;
}


/* PIPE */

.svgPipe{
    stroke:#596971;
    stroke-width:8;

    fill:none;

    transition:.25s;
}

.running .svgPipe{
    stroke:#26986a;

    filter:
      drop-shadow(0 0 3px rgba(38,152,106,.6));
}


/* MOTOR */

.motorHousing{
    fill:#1c272d;

    stroke:#7c8b92;
    stroke-width:2;

    transition:.25s;
}

.running .motorHousing{
    stroke:#38d58f;
}


/* MOTOR COOLING FINS */

.motorFin{
    stroke:#59676e;
    stroke-width:2;
}

.running .motorFin{
    stroke:#26986a;
}


/* SHAFT */

.shaft{
    fill:#78878e;
}


/* PUMP VOLUTE */

.volute{
    fill:#151f24;

    stroke:#83939a;
    stroke-width:3;

    transition:.25s;
}

.running .volute{
    stroke:#39e49a;

    filter:
      drop-shadow(0 0 4px rgba(57,228,154,.45));
}


/* IMPELLER */

.impeller{
    fill:none;

    stroke:#697980;
    stroke-width:3;

    transform-origin:77px 74px;
}

.running .impeller{
    stroke:#3ce69d;

    animation:spin .75s linear infinite;
}

@keyframes spin{
    from{transform:rotate(0deg);}
    to{transform:rotate(360deg);}
}


/* FLOW ANIMATION */

.flowDash{
    stroke:#3ce69d;
    stroke-width:3;

    stroke-dasharray:5 12;

    fill:none;

    opacity:0;
}

.running .flowDash{
    opacity:1;

    animation:flow 1s linear infinite;
}

@keyframes flow{
    to{
      stroke-dashoffset:-34;
    }
}


/* ==========================================================
   EQUIPMENT INFORMATION
   ========================================================== */

.infoGrid{
    display:grid;

    grid-template-columns:1fr 1fr;

    gap:7px;

    padding:10px 0 12px;

    border-top:1px solid #273238;
    border-bottom:1px solid #273238;

    font-family:monospace;
}

.infoBox{
    background:#090e11;

    border:1px solid #202b31;

    padding:7px;
}

.infoLabel{
    color:#52626a;

    font-size:8px;

    margin-bottom:3px;
}

.infoValue{
    color:#aab7bd;

    font-size:10px;
}


/* ==========================================================
   BUTTONS
   ========================================================== */

.controls{
    display:grid;

    grid-template-columns:1fr 1fr;

    gap:8px;

    margin-top:13px;
}

.controlBtn{
    height:42px;

    border-radius:2px;

    font-family:Arial,Helvetica,sans-serif;
    font-weight:bold;
    font-size:11px;
    letter-spacing:1px;

    cursor:pointer;

    transition:
      transform .08s,
      filter .15s;

    -webkit-tap-highlight-color:transparent;
}

.controlBtn:active{
    transform:translateY(2px);
}

.startBtn{
    color:#a9ffcf;

    background:
      linear-gradient(#174f39,#0c3022);

    border:1px solid #25865d;

    box-shadow:
      inset 0 1px 0 rgba(255,255,255,.08);
}

.stopBtn{
    color:#ffb3b3;

    background:
      linear-gradient(#562525,#321414);

    border:1px solid #8c3939;

    box-shadow:
      inset 0 1px 0 rgba(255,255,255,.06);
}

.controlBtn:hover{
    filter:brightness(1.18);
}


/* ==========================================================
   LOWER CONTROL PANEL
   ========================================================== */

master{}

.masterPanel{
    margin-top:18px;

    display:grid;

    grid-template-columns:1fr auto;

    gap:18px;

    background:
      linear-gradient(#141b1f,#0c1114);

    border:1px solid #303b41;

    padding:16px 18px;
}

.relayRack{
    display:flex;

    align-items:center;

    gap:15px;

    flex-wrap:wrap;
}

.rackTitle{
    font-family:monospace;

    font-size:10px;

    color:#687982;

    margin-right:5px;
}

.relay{
    min-width:95px;

    padding:8px 10px;

    border:1px solid #28353b;

    background:#090e11;

    display:flex;

    align-items:center;

    gap:8px;
}

.relayLamp{
    width:8px;
    height:8px;

    border-radius:50%;

    background:#6c2929;
}

.relay.active .relayLamp{
    background:#2ce58b;

    box-shadow:
      0 0 7px rgba(44,229,139,.8);
}

.relayText{
    font-family:monospace;

    font-size:9px;

    color:#819098;
}


/* MASTER BUTTONS */

.masterControls{
    display:flex;

    gap:9px;
}

masterButton{}

.masterButton{
    min-width:110px;

    padding:11px 16px;

    font-size:10px;

    font-weight:bold;

    letter-spacing:1px;

    border-radius:2px;

    cursor:pointer;
}

.allStart{
    background:#123c2c;

    border:1px solid #247653;

    color:#83e9b3;
}

.allStop{
    background:#581d1d;

    border:1px solid #a63a3a;

    color:#ffaaaa;
}


/* ==========================================================
   FOOTER
   ========================================================== */

.footer{
    display:flex;

    justify-content:space-between;

    margin-top:10px;

    font-family:monospace;

    color:#46545b;

    font-size:9px;

    letter-spacing:.5px;
}


/* ==========================================================
   MOBILE
   ========================================================== */

@media(max-width:950px){

    .pumps{
        grid-template-columns:
          repeat(2,1fr);
    }

}

@media(max-width:600px){

    .topbar{
        height:auto;

        padding:15px;

        gap:10px;
    }

    .title{
        font-size:13px;
    }

    .subtitle{
        font-size:8px;
    }

    .systemStatus{
        font-size:9px;
        padding:7px;
    }

    .panel{
        margin-top:15px;
        padding:0 10px 25px;
    }

    .processPanel{
        padding:20px 10px;
    }

    .pumps{
        grid-template-columns:1fr;
    }

    .pumpModule{
        min-height:410px;
    }

    .pipeHeader{
        display:none;
    }

    .diagramTitle{
        margin-bottom:15px;
    }

    .masterPanel{
        grid-template-columns:1fr;
    }

    .masterControls{
        width:100%;
    }

    .masterButton{
        flex:1;
    }

    .footer{
        flex-direction:column;
        gap:4px;
    }
}

</style>
</head>

<body>

<!-- ========================================================
     HEADER
     ======================================================== -->

<div class="topbar">

    <div class="brand">

        <div class="logo"></div>

        <div>
            <div class="title">
                WATER PUMP CONTROL SYSTEM
            </div>

            <div class="subtitle">
                ESP32 INDUSTRIAL CONTROL NODE
            </div>
        </div>

    </div>

    <div class="systemStatus">

        <span class="onlineLamp"></span>

        <span id="systemText">
            SYSTEM ONLINE
        </span>

    </div>

</div>


<!-- ========================================================
     MAIN PANEL
     ======================================================== -->

<div class="panel">

    <div class="panelHeader">

        <div class="panelTitle">
            PROCESS CONTROL / PUMP STATION
        </div>

        <div class="panelSmall">
            MODE: REMOTE
        </div>

    </div>


    <div class="processPanel">

        <div class="bolt b1"></div>
        <div class="bolt b2"></div>
        <div class="bolt b3"></div>
        <div class="bolt b4"></div>

        <div class="diagramTitle">
            WATER DISTRIBUTION MANIFOLD // P-101 - P-104
        </div>


        <!-- Main schematic pipe -->

        <div class="pipeHeader">

            <div class="mainPipe"></div>

            <div class="flowArrow"></div>

        </div>


        <!-- ==================================================
             PUMPS
             ================================================== -->

        <div class="pumps">


<!-- ======================== PUMP 1 ======================== -->

<div class="pumpModule" id="pump1">

    <div class="moduleTop">

        <div class="equipmentID">
            P-101
        </div>

        <div class="channel">
            RELAY CH-01
        </div>

    </div>

    <div class="statusArea">

        <div class="statusLamp"></div>

        <div class="statusText" id="status1">
            STOPPED
        </div>

    </div>


    <div class="pumpDrawing">

        <svg class="pumpSvg"
             viewBox="0 0 180 145">


            <!-- INLET PIPE -->

            <path
              class="svgPipe"
              d="M0 75 H48"/>


            <!-- OUTLET PIPE -->

            <path
              class="svgPipe"
              d="M76 47 V20 H110"/>


            <!-- animated inlet flow -->

            <path
              class="flowDash"
              d="M0 75 H48"/>


            <!-- animated outlet flow -->

            <path
              class="flowDash"
              d="M76 47 V20 H110"/>


            <!-- centrifugal pump casing -->

            <circle
              class="volute"
              cx="76"
              cy="75"
              r="29"/>


            <!-- volute discharge detail -->

            <path
              class="volute"
              d="M77 46
                 C101 44 111 57 110 74
                 L96 74
                 C97 64 91 58 78 59
                 Z"/>


            <!-- impeller -->

            <g class="impeller">

                <circle
                  cx="76"
                  cy="75"
                  r="7"/>

                <path d="M76 68 Q88 60 91 69"/>

                <path d="M83 75 Q91 87 82 91"/>

                <path d="M76 82 Q64 91 61 82"/>

                <path d="M69 75 Q61 63 70 60"/>

            </g>


            <!-- shaft -->

            <rect
              class="shaft"
              x="103"
              y="71"
              width="18"
              height="8"/>


            <!-- electric motor -->

            <rect
              class="motorHousing"
              x="120"
              y="53"
              width="51"
              height="44"
              rx="5"/>


            <!-- cooling fins -->

            <line class="motorFin"
                  x1="129" y1="57"
                  x2="129" y2="93"/>

            <line class="motorFin"
                  x1="137" y1="57"
                  x2="137" y2="93"/>

            <line class="motorFin"
                  x1="145" y1="57"
                  x2="145" y2="93"/>

            <line class="motorFin"
                  x1="153" y1="57"
                  x2="153" y2="93"/>

            <line class="motorFin"
                  x1="161" y1="57"
                  x2="161" y2="93"/>


            <!-- motor base -->

            <path
              class="motorHousing"
              d="M125 97 H165 L170 106 H120 Z"/>

        </svg>

    </div>


    <div class="infoGrid">

        <div class="infoBox">
            <div class="infoLabel">
                CONTROLLER
            </div>

            <div class="infoValue">
                GPIO 14
            </div>
        </div>

        <div class="infoBox">
            <div class="infoLabel">
                DRIVE
            </div>

            <div class="infoValue">
                RELAY 01
            </div>
        </div>

    </div>


    <div class="controls">

        <button
          class="controlBtn startBtn"
          onclick="setPump(1,1)">
          START
        </button>

        <button
          class="controlBtn stopBtn"
          onclick="setPump(1,0)">
          STOP
        </button>

    </div>

</div>



<!-- ======================== PUMP 2 ======================== -->

<div class="pumpModule" id="pump2">

    <div class="moduleTop">
        <div class="equipmentID">P-102</div>
        <div class="channel">RELAY CH-02</div>
    </div>

    <div class="statusArea">
        <div class="statusLamp"></div>
        <div class="statusText" id="status2">
            STOPPED
        </div>
    </div>

    <div class="pumpDrawing">

        <svg class="pumpSvg"
             viewBox="0 0 180 145">

            <path class="svgPipe"
                  d="M0 75 H48"/>

            <path class="svgPipe"
                  d="M76 47 V20 H110"/>

            <path class="flowDash"
                  d="M0 75 H48"/>

            <path class="flowDash"
                  d="M76 47 V20 H110"/>

            <circle class="volute"
                    cx="76"
                    cy="75"
                    r="29"/>

            <path class="volute"
                  d="M77 46
                     C101 44 111 57 110 74
                     L96 74
                     C97 64 91 58 78 59 Z"/>

            <g class="impeller">

                <circle cx="76"
                        cy="75"
                        r="7"/>

                <path d="M76 68 Q88 60 91 69"/>
                <path d="M83 75 Q91 87 82 91"/>
                <path d="M76 82 Q64 91 61 82"/>
                <path d="M69 75 Q61 63 70 60"/>

            </g>

            <rect class="shaft"
                  x="103"
                  y="71"
                  width="18"
                  height="8"/>

            <rect class="motorHousing"
                  x="120"
                  y="53"
                  width="51"
                  height="44"
                  rx="5"/>

            <line class="motorFin"
                  x1="129" y1="57"
                  x2="129" y2="93"/>

            <line class="motorFin"
                  x1="137" y1="57"
                  x2="137" y2="93"/>

            <line class="motorFin"
                  x1="145" y1="57"
                  x2="145" y2="93"/>

            <line class="motorFin"
                  x1="153" y1="57"
                  x2="153" y2="93"/>

            <line class="motorFin"
                  x1="161" y1="57"
                  x2="161" y2="93"/>

            <path class="motorHousing"
                  d="M125 97 H165 L170 106 H120 Z"/>

        </svg>

    </div>


    <div class="infoGrid">

        <div class="infoBox">
            <div class="infoLabel">CONTROLLER</div>
            <div class="infoValue">GPIO 27</div>
        </div>

        <div class="infoBox">
            <div class="infoLabel">DRIVE</div>
            <div class="infoValue">RELAY 02</div>
        </div>

    </div>


    <div class="controls">

        <button class="controlBtn startBtn"
                onclick="setPump(2,1)">
            START
        </button>

        <button class="controlBtn stopBtn"
                onclick="setPump(2,0)">
            STOP
        </button>

    </div>

</div>



<!-- ======================== PUMP 3 ======================== -->

<div class="pumpModule" id="pump3">

    <div class="moduleTop">
        <div class="equipmentID">P-103</div>
        <div class="channel">RELAY CH-03</div>
    </div>

    <div class="statusArea">
        <div class="statusLamp"></div>
        <div class="statusText" id="status3">
            STOPPED
        </div>
    </div>

    <div class="pumpDrawing">

        <svg class="pumpSvg"
             viewBox="0 0 180 145">

            <path class="svgPipe"
                  d="M0 75 H48"/>

            <path class="svgPipe"
                  d="M76 47 V20 H110"/>

            <path class="flowDash"
                  d="M0 75 H48"/>

            <path class="flowDash"
                  d="M76 47 V20 H110"/>

            <circle class="volute"
                    cx="76"
                    cy="75"
                    r="29"/>

            <path class="volute"
                  d="M77 46
                     C101 44 111 57 110 74
                     L96 74
                     C97 64 91 58 78 59 Z"/>

            <g class="impeller">

                <circle cx="76"
                        cy="75"
                        r="7"/>

                <path d="M76 68 Q88 60 91 69"/>
                <path d="M83 75 Q91 87 82 91"/>
                <path d="M76 82 Q64 91 61 82"/>
                <path d="M69 75 Q61 63 70 60"/>

            </g>

            <rect class="shaft"
                  x="103"
                  y="71"
                  width="18"
                  height="8"/>

            <rect class="motorHousing"
                  x="120"
                  y="53"
                  width="51"
                  height="44"
                  rx="5"/>

            <line class="motorFin"
                  x1="129" y1="57"
                  x2="129" y2="93"/>

            <line class="motorFin"
                  x1="137" y1="57"
                  x2="137" y2="93"/>

            <line class="motorFin"
                  x1="145" y1="57"
                  x2="145" y2="93"/>

            <line class="motorFin"
                  x1="153" y1="57"
                  x2="153" y2="93"/>

            <line class="motorFin"
                  x1="161" y1="57"
                  x2="161" y2="93"/>

            <path class="motorHousing"
                  d="M125 97 H165 L170 106 H120 Z"/>

        </svg>

    </div>


    <div class="infoGrid">

        <div class="infoBox">
            <div class="infoLabel">CONTROLLER</div>
            <div class="infoValue">GPIO 26</div>
        </div>

        <div class="infoBox">
            <div class="infoLabel">DRIVE</div>
            <div class="infoValue">RELAY 03</div>
        </div>

    </div>


    <div class="controls">

        <button class="controlBtn startBtn"
                onclick="setPump(3,1)">
            START
        </button>

        <button class="controlBtn stopBtn"
                onclick="setPump(3,0)">
            STOP
        </button>

    </div>

</div>



<!-- ======================== PUMP 4 ======================== -->

<div class="pumpModule" id="pump4">

    <div class="moduleTop">
        <div class="equipmentID">P-104</div>
        <div class="channel">RELAY CH-04</div>
    </div>

    <div class="statusArea">
        <div class="statusLamp"></div>
        <div class="statusText" id="status4">
            STOPPED
        </div>
    </div>

    <div class="pumpDrawing">

        <svg class="pumpSvg"
             viewBox="0 0 180 145">

            <path class="svgPipe"
                  d="M0 75 H48"/>

            <path class="svgPipe"
                  d="M76 47 V20 H110"/>

            <path class="flowDash"
                  d="M0 75 H48"/>

            <path class="flowDash"
                  d="M76 47 V20 H110"/>

            <circle class="volute"
                    cx="76"
                    cy="75"
                    r="29"/>

            <path class="volute"
                  d="M77 46
                     C101 44 111 57 110 74
                     L96 74
                     C97 64 91 58 78 59 Z"/>

            <g class="impeller">

                <circle cx="76"
                        cy="75"
                        r="7"/>

                <path d="M76 68 Q88 60 91 69"/>
                <path d="M83 75 Q91 87 82 91"/>
                <path d="M76 82 Q64 91 61 82"/>
                <path d="M69 75 Q61 63 70 60"/>

            </g>

            <rect class="shaft"
                  x="103"
                  y="71"
                  width="18"
                  height="8"/>

            <rect class="motorHousing"
                  x="120"
                  y="53"
                  width="51"
                  height="44"
                  rx="5"/>

            <line class="motorFin"
                  x1="129" y1="57"
                  x2="129" y2="93"/>

            <line class="motorFin"
                  x1="137" y1="57"
                  x2="137" y2="93"/>

            <line class="motorFin"
                  x1="145" y1="57"
                  x2="145" y2="93"/>

            <line class="motorFin"
                  x1="153" y1="57"
                  x2="153" y2="93"/>

            <line class="motorFin"
                  x1="161" y1="57"
                  x2="161" y2="93"/>

            <path class="motorHousing"
                  d="M125 97 H165 L170 106 H120 Z"/>

        </svg>

    </div>


    <div class="infoGrid">

        <div class="infoBox">
            <div class="infoLabel">CONTROLLER</div>
            <div class="infoValue">GPIO 25</div>
        </div>

        <div class="infoBox">
            <div class="infoLabel">DRIVE</div>
            <div class="infoValue">RELAY 04</div>
        </div>

    </div>


    <div class="controls">

        <button class="controlBtn startBtn"
                onclick="setPump(4,1)">
            START
        </button>

        <button class="controlBtn stopBtn"
                onclick="setPump(4,0)">
            STOP
        </button>

    </div>

</div>


        </div>
    </div>


<!-- ========================================================
     MASTER CONTROL
     ======================================================== -->

<div class="masterPanel">

    <div class="relayRack">

        <div class="rackTitle">
            OUTPUT RELAYS
        </div>


        <div class="relay"
             id="relay1">

            <div class="relayLamp"></div>

            <div class="relayText">
                K1 / OFF
            </div>

        </div>


        <div class="relay"
             id="relay2">

            <div class="relayLamp"></div>

            <div class="relayText">
                K2 / OFF
            </div>

        </div>


        <div class="relay"
             id="relay3">

            <div class="relayLamp"></div>

            <div class="relayText">
                K3 / OFF
            </div>

        </div>


        <div class="relay"
             id="relay4">

            <div class="relayLamp"></div>

            <div class="relayText">
                K4 / OFF
            </div>

        </div>

    </div>


    <div class="masterControls">

        <button
          class="masterButton allStart"
          onclick="setAll(1)">
            START ALL
        </button>

        <button
          class="masterButton allStop"
          onclick="setAll(0)">
            STOP ALL
        </button>

    </div>

</div>


<div class="footer">

    <span>
        CTRL: ESP32 // NETWORK CONTROL ENABLED
    </span>

    <span id="connectionStatus">
        LINK: CONNECTED
    </span>

</div>


</div>


<!-- ========================================================
     JAVASCRIPT
     ======================================================== -->

<script>

let pumpStates = [false,false,false,false];


/* ----------------------------------------------------------
   SET INDIVIDUAL PUMP
   ---------------------------------------------------------- */

async function setPump(pump,state){

    try{

        const response =
          await fetch(
            "/pump?id=" +
            pump +
            "&state=" +
            state,
            {cache:"no-store"}
          );

        if(!response.ok){
            throw new Error("HTTP error");
        }

        const result =
          await response.text();

        pumpStates[pump-1] =
          result.trim() === "1";

        updateUI();

        setConnection(true);

    }
    catch(error){

        console.log(error);

        setConnection(false);

    }

}


/* ----------------------------------------------------------
   MASTER CONTROL
   ---------------------------------------------------------- */

async function setAll(state){

    try{

        const response =
          await fetch(
            "/all?state=" + state,
            {cache:"no-store"}
          );

        if(!response.ok){
            throw new Error("HTTP error");
        }

        await response.text();

        for(let i=0;i<4;i++){
            pumpStates[i] =
              state === 1;
        }

        updateUI();

        setConnection(true);

    }
    catch(error){

        console.log(error);

        setConnection(false);

    }

}


/* ----------------------------------------------------------
   UPDATE GRAPHICS
   ---------------------------------------------------------- */

function updateUI(){

    for(let i=0;i<4;i++){

        const pumpNumber = i+1;

        const module =
          document.getElementById(
            "pump"+pumpNumber
          );

        const status =
          document.getElementById(
            "status"+pumpNumber
          );

        const relay =
          document.getElementById(
            "relay"+pumpNumber
          );

        const relayText =
          relay.querySelector(
            ".relayText"
          );


        if(pumpStates[i]){

            module.classList.add(
              "running"
            );

            relay.classList.add(
              "active"
            );

            status.textContent =
              "RUNNING";

            relayText.textContent =
              "K"+pumpNumber+" / ON";

        }
        else{

            module.classList.remove(
              "running"
            );

            relay.classList.remove(
              "active"
            );

            status.textContent =
              "STOPPED";

            relayText.textContent =
              "K"+pumpNumber+" / OFF";

        }

    }

}


/* ----------------------------------------------------------
   READ REAL ESP32 STATUS
   ---------------------------------------------------------- */

async function getStatus(){

    try{

        const response =
          await fetch(
            "/status?t="+Date.now(),
            {cache:"no-store"}
          );

        if(!response.ok){
            throw new Error(
              "Connection failed"
            );
        }

        const data =
          await response.json();

        pumpStates = data;

        updateUI();

        setConnection(true);

    }
    catch(error){

        setConnection(false);

    }

}


/* ----------------------------------------------------------
   CONNECTION STATUS
   ---------------------------------------------------------- */

function setConnection(connected){

    const bottom =
      document.getElementById(
        "connectionStatus"
      );

    const top =
      document.getElementById(
        "systemText"
      );


    if(connected){

        bottom.textContent =
          "LINK: CONNECTED";

        top.textContent =
          "SYSTEM ONLINE";

    }
    else{

        bottom.textContent =
          "LINK: LOST";

        top.textContent =
          "CONNECTION LOST";

    }

}


/* Initial sync */

getStatus();


/* Periodic synchronization */

setInterval(
  getStatus,
  2500
);

</script>

</body>
</html>
)rawliteral";


// ============================================================
// SEND MAIN PAGE
// ============================================================

void handleRoot(){

    server.sendHeader(
      "Cache-Control",
      "no-cache, no-store, must-revalidate"
    );

    server.sendHeader(
      "Pragma",
      "no-cache"
    );

    server.send_P(
      200,
      "text/html; charset=utf-8",
      INDEX_HTML
    );
}


// ============================================================
// CONTROL INDIVIDUAL PUMP
// URL:
// /pump?id=1&state=1
// ============================================================

void handlePump(){

    if(
      !server.hasArg("id") ||
      !server.hasArg("state")
    ){

        server.send(
          400,
          "text/plain",
          "Missing parameters"
        );

        return;
    }


    int pump =
      server.arg("id").toInt();

    int requestedState =
      server.arg("state").toInt();


    if(pump < 1 || pump > 4){

        server.send(
          400,
          "text/plain",
          "Invalid pump"
        );

        return;
    }


    if(
      requestedState != 0 &&
      requestedState != 1
    ){

        server.send(
          400,
          "text/plain",
          "Invalid state"
        );

        return;
    }


    int index = pump - 1;

    pumpState[index] =
      requestedState == 1;


    digitalWrite(
      relayPins[index],
      pumpState[index]
        ? RELAY_ON
        : RELAY_OFF
    );


    Serial.print("P-10");
    Serial.print(pump);

    Serial.print(" -> ");

    Serial.println(
      pumpState[index]
        ? "RUNNING"
        : "STOPPED"
    );


    server.send(
      200,
      "text/plain",
      pumpState[index]
        ? "1"
        : "0"
    );

}


// ============================================================
// ALL PUMPS
// ============================================================

void handleAll(){

    if(!server.hasArg("state")){

        server.send(
          400,
          "text/plain",
          "Missing state"
        );

        return;
    }


    int requestedState =
      server.arg("state").toInt();


    if(
      requestedState != 0 &&
      requestedState != 1
    ){

        server.send(
          400,
          "text/plain",
          "Invalid state"
        );

        return;
    }


    bool newState =
      requestedState == 1;


    for(int i=0;i<4;i++){

        pumpState[i] =
          newState;

        digitalWrite(
          relayPins[i],
          newState
            ? RELAY_ON
            : RELAY_OFF
        );

    }


    Serial.println(
      newState
        ? "ALL PUMPS RUNNING"
        : "ALL PUMPS STOPPED"
    );


    server.send(
      200,
      "text/plain",
      "OK"
    );

}


// ============================================================
// RETURN STATUS
// ============================================================

void handleStatus(){

    String json = "[";

    for(int i=0;i<4;i++){

        json +=
          pumpState[i]
            ? "true"
            : "false";

        if(i<3){
            json += ",";
        }

    }

    json += "]";


    server.sendHeader(
      "Cache-Control",
      "no-cache"
    );


    server.send(
      200,
      "application/json",
      json
    );

}


// ============================================================
// 404
// ============================================================

void handleNotFound(){

    server.send(
      404,
      "text/plain",
      "404"
    );

}


// ============================================================
// SETUP
// ============================================================

void setup(){

    Serial.begin(115200);

    delay(500);


    Serial.println();
    Serial.println(
      "=============================="
    );

    Serial.println(
      "ESP32 PUMP CONTROL SYSTEM"
    );

    Serial.println(
      "=============================="
    );


    // --------------------------------------------------------
    // RELAYS
    // --------------------------------------------------------

    // Make sure pumps are OFF at startup.

    for(int i=0;i<4;i++){

        pinMode(
          relayPins[i],
          OUTPUT
        );

        digitalWrite(
          relayPins[i],
          RELAY_OFF
        );

        pumpState[i] = false;

    }


    // --------------------------------------------------------
    // WIFI
    // --------------------------------------------------------

    WiFi.mode(WIFI_STA);

    WiFi.setSleep(false);

    WiFi.begin(
      ssid,
      password
    );


    Serial.print(
      "Connecting to WiFi"
    );


    while(
      WiFi.status() != WL_CONNECTED
    ){

        delay(500);

        Serial.print(".");

    }


    Serial.println();
    Serial.println();

    Serial.println(
      "WiFi connected."
    );


    Serial.print(
      "IP ADDRESS: "
    );

    Serial.println(
      WiFi.localIP()
    );


    // --------------------------------------------------------
    // SERVER ROUTES
    // --------------------------------------------------------

    server.on(
      "/",
      HTTP_GET,
      handleRoot
    );


    server.on(
      "/pump",
      HTTP_GET,
      handlePump
    );


    server.on(
      "/all",
      HTTP_GET,
      handleAll
    );


    server.on(
      "/status",
      HTTP_GET,
      handleStatus
    );


    server.onNotFound(
      handleNotFound
    );


    server.begin();


    Serial.println(
      "Web control server started."
    );

    Serial.println(
      "Open the IP address above."
    );

    Serial.println(
      "=============================="
    );

}


// ============================================================
// LOOP
// ============================================================

void loop(){

    server.handleClient();

}