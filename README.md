# 🔔 Timbre Automático ESP32 - Guía Completa de Instalación

Sistema de timbre automático con ESP32, pantalla OLED, interfaz web y horarios programables.

## 📋 Tabla de Contenidos

- [Requisitos](#requisitos)
- [Hardware](#hardware)
- [Software](#software)
- [Instalación Paso a Paso](#instalación-paso-a-paso)
- [Configuración Arduino IDE](#configuración-arduino-ide)
- [Creación de Archivos](#creación-de-archivos)
- [Configuración LittleFS](#configuración-littlefs)
- [Subida del Código](#subida-del-código)
- [Uso del Sistema](#uso-del-sistema)
- [Solución de Problemas](#solución-de-problemas)

## 🔧 Requisitos

### Hardware

- **ESP32** (cualquier modelo compatible)
- **Pantalla OLED SSD1306** 128x64 (opcional)
- **Módulo Relé** 5V/3.3V
- **Timbre/Chicharra** (12V o el que tengas)
- **Fuente de alimentación** 5V/12V
- **Cables jumper**
- **Breadboard o PCB**

### Software

- **Arduino IDE 2.x** (versión 2.3.6 o superior)
- **Python** (para esptool)
- **PowerShell** (Windows)

## ⚡ Hardware

### Conexiones ESP32

```
ESP32 Pin  |  Componente     |  Descripción
-----------|-----------------|------------------
GPIO 2     |  Relé IN        |  Señal control relé
GPIO 21    |  OLED SDA       |  Datos I2C (opcional)
GPIO 22    |  OLED SCL       |  Reloj I2C (opcional)
3.3V       |  OLED VCC       |  Alimentación OLED
GND        |  OLED GND       |  Tierra OLED
GND        |  Relé GND       |  Tierra relé
5V/3.3V    |  Relé VCC       |  Alimentación relé
```

### Esquema del Relé

```
ESP32 GPIO2 ──→ Relé IN
ESP32 GND   ──→ Relé GND
ESP32 5V    ──→ Relé VCC

Relé COM    ──→ Timbre +
Relé NO     ──→ Fuente 12V +
Fuente 12V- ──→ Timbre -
```

## 💻 Software

### 1. Instalar Arduino IDE 2.x

1. Descarga desde: https://www.arduino.cc/en/software
2. Instala la versión **Arduino IDE 2.3.6** o superior

### 2. Configurar ESP32 en Arduino IDE

1. **Abrir Arduino IDE**
2. **File → Preferences**
3. **Additional Boards Manager URLs:** agregar:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. **Tools → Board → Boards Manager**
5. **Buscar "ESP32"** por Espressif Systems
6. **Install** la última versión (2.0.x o superior)

### 3. Instalar Librerías

**Tools → Manage Libraries**, buscar e instalar:

- `Adafruit GFX Library`
- `Adafruit SSD1306`
- `ArduinoJson` (by Benoit Blanchon)
- `RTClib` (by Adafruit) - opcional

## 🚀 Instalación Paso a Paso

### Paso 1: Crear Proyecto

```bash
# Crear carpeta del proyecto
mkdir timbre_esp32
cd timbre_esp32
```

### Paso 2: Crear Archivo Principal

**Crear archivo `timbre.ino`** y copiar el código principal:

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ... [CÓDIGO COMPLETO DEL TIMBRE] ...
```

### Paso 3: Crear Carpeta data/

```bash
# En la carpeta del proyecto
mkdir data
```

### Paso 4: Crear Archivos Web

#### data/index.html

```html
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Timbre Automático 23 de Marzo</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔔 Timbre Automático</h1>
            <h2>23 de Marzo</h2>
            <div class="hora-actual" id="horaActual">Cargando...</div>
        </div>
        
        <div class="content">
            <div class="estado-sistema" id="estadoSistema">
                Sistema Activo ✓
            </div>
            
            <div class="logs-container">
                <h3>📋 Último Log</h3>
                <div class="log-display" id="logDisplay">
                    Esperando logs...
                </div>
            </div>
            
            <div class="controles-principales">
                <button class="btn btn-primary" onclick="sonarManual()">
                    🔔 SONAR TIMBRE
                </button>
                <button class="btn btn-danger" onclick="toggleSistema()" id="btnToggle">
                    ❌ DESACTIVAR SISTEMA
                </button>
            </div>
            
            <div class="seccion">
                <h2>📅 DÍAS DE LA SEMANA</h2>
                <div class="dias-semana" id="diasSemana"></div>
                <button class="btn btn-success" onclick="guardarDias()">
                    💾 GUARDAR DÍAS
                </button>
            </div>
            
            <div class="seccion">
                <h2>⏰ TIPOS DE HORARIOS</h2>
                <div class="form-group">
                    <label>Crear Nuevo Tipo de Horario:</label>
                    <div class="input-group">
                        <input type="text" id="nuevoTipo" placeholder="Ej: Recreos, Clases, Entrada, Salida...">
                        <button class="btn btn-primary" onclick="crearTipoHorario()">
                            ➕ CREAR
                        </button>
                    </div>
                </div>
                <div class="tipos-horarios" id="tiposHorarios"></div>
            </div>
        </div>
    </div>
    <script src="/app.js"></script>
</body>
</html>
```

#### data/style.css

```css
/* CSS COMPACTO PARA EL TIMBRE */
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:Arial,sans-serif;background:linear-gradient(135deg,#667eea,#764ba2);min-height:100vh;padding:20px}
.container{max-width:1200px;margin:0 auto;background:white;border-radius:15px;box-shadow:0 20px 40px rgba(0,0,0,0.1);overflow:hidden}
.header{background:linear-gradient(45deg,#FF6B6B,#4ECDC4);color:white;padding:30px;text-align:center}
.header h1{font-size:2.5em;margin-bottom:10px}
.hora-actual{font-size:1.5em;margin-top:15px;padding:15px;background:rgba(255,255,255,0.2);border-radius:10px;display:inline-block}
.content{padding:30px}
.logs-container{background:#f8f9fa;border-radius:10px;padding:20px;margin:20px 0;border-left:5px solid #17a2b8}
.log-display{background:#1a1a1a;color:#00ff00;padding:15px;border-radius:8px;font-family:monospace;font-size:14px;min-height:40px}
.controles-principales{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px;margin-bottom:30px}
.btn{padding:15px 25px;border:none;border-radius:10px;font-size:1.1em;cursor:pointer;transition:all 0.3s ease;font-weight:bold}
.btn-primary{background:linear-gradient(45deg,#4ECDC4,#44A08D);color:white}
.btn-danger{background:linear-gradient(45deg,#FF6B6B,#FF8E53);color:white}
.btn-success{background:linear-gradient(45deg,#45B7D1,#96C93D);color:white}
.btn:hover{transform:translateY(-3px);box-shadow:0 8px 25px rgba(0,0,0,0.2)}
.estado-sistema{text-align:center;padding:20px;margin:20px 0;border-radius:15px;font-size:1.3em;font-weight:bold}
.sistema-activo{background:linear-gradient(45deg,#d4edda,#c3e6cb);color:#155724;border:2px solid #28a745}
.sistema-inactivo{background:linear-gradient(45deg,#f8d7da,#f5c6cb);color:#721c24;border:2px solid #dc3545}
.seccion{margin:30px 0;padding:25px;border-radius:15px;background:#f8f9fa;border-left:5px solid #4ECDC4}
.seccion h2{color:#333;margin-bottom:20px;font-size:1.8em}
.form-group{margin:15px 0}
.input-group{display:flex;gap:10px;flex-wrap:wrap}
.form-group input{flex:1;min-width:200px;padding:12px;border:2px solid #ddd;border-radius:8px;font-size:1em}
.tipos-horarios{margin-top:20px}
.tipo-horario{background:white;border-radius:15px;padding:25px;margin:20px 0;border:2px solid #e9ecef}
.tipo-header{display:flex;justify-content:space-between;align-items:center;margin-bottom:20px}
.tipo-nombre{font-size:1.4em;font-weight:bold;color:#333}
.horarios-lista{display:grid;grid-template-columns:repeat(auto-fill,minmax(140px,1fr));gap:12px;margin:20px 0}
.horario-item{background:linear-gradient(45deg,#e3f2fd,#bbdefb);padding:12px;border-radius:8px;display:flex;justify-content:space-between;align-items:center;font-weight:bold}
.btn-small{padding:6px 10px;font-size:0.8em;border-radius:6px}
.dias-semana{display:grid;grid-template-columns:repeat(auto-fit,minmax(140px,1fr));gap:15px;margin:20px 0}
.dia-checkbox{display:flex;align-items:center;padding:15px;background:white;border-radius:10px;border:2px solid #ddd;cursor:pointer;font-weight:bold}
.dia-checkbox input{margin-right:10px;transform:scale(1.3)}
.dia-activo{background:linear-gradient(45deg,#d4edda,#c3e6cb);border-color:#28a745;color:#155724}
.toggle-switch{position:relative;display:inline-block;width:70px;height:40px}
.toggle-switch input{opacity:0;width:0;height:0}
.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;transition:.4s;border-radius:40px}
.slider:before{position:absolute;content:"";height:32px;width:32px;left:4px;bottom:4px;background-color:white;transition:.4s;border-radius:50%}
input:checked + .slider{background:linear-gradient(45deg,#4ECDC4,#44A08D)}
input:checked + .slider:before{transform:translateX(30px)}
```

#### data/app.js

```javascript
// JAVASCRIPT COMPACTO PARA EL TIMBRE
let sistemaActivo=true;let estadoDias=[true,true,true,true,true,true,true];let tiposHorarios=[];const diasNombres=['Lunes','Martes','Miércoles','Jueves','Viernes','Sábado','Domingo'];window.onload=function(){actualizarHora();cargarEstado();setInterval(actualizarHora,1000);setInterval(cargarEstado,5000);setInterval(cargarLogs,2000)};async function actualizarHora(){try{const response=await fetch('/api/hora');const data=await response.json();document.getElementById('horaActual').innerHTML=`📅 ${data.fecha} ⏰ ${data.hora}`}catch(error){document.getElementById('horaActual').innerHTML='❌ Error de conexión'}}async function cargarEstado(){try{const response=await fetch('/api/estado');const data=await response.json();sistemaActivo=data.sistemaActivo;estadoDias=data.dias;tiposHorarios=data.tipos;actualizarInterfaz()}catch(error){console.error('Error al cargar estado:',error)}}async function cargarLogs(){try{const response=await fetch('/api/logs');const data=await response.json();if(data.ultimoLog){const timestamp=new Date(data.timestamp).toLocaleTimeString();document.getElementById('logDisplay').innerHTML=`[${timestamp}] ${data.ultimoLog}`}}catch(error){console.error('Error al cargar logs:',error)}}function actualizarInterfaz(){const estadoEl=document.getElementById('estadoSistema');const btnToggle=document.getElementById('btnToggle');if(sistemaActivo){estadoEl.textContent='✅ Sistema Activo - Funcionando Correctamente';estadoEl.className='estado-sistema sistema-activo';btnToggle.innerHTML='❌ DESACTIVAR SISTEMA';btnToggle.className='btn btn-danger'}else{estadoEl.textContent='⚠️ Sistema Inactivo - Timbre Deshabilitado';estadoEl.className='estado-sistema sistema-inactivo';btnToggle.innerHTML='✅ ACTIVAR SISTEMA';btnToggle.className='btn btn-success'}actualizarDias();actualizarTiposHorarios()}function actualizarDias(){const container=document.getElementById('diasSemana');container.innerHTML='';for(let i=0;i<7;i++){const div=document.createElement('div');div.className=`dia-checkbox ${estadoDias[i]?'dia-activo':''}`;div.innerHTML=`<input type="checkbox" ${estadoDias[i]?'checked':''} onchange="estadoDias[${i}] = this.checked; actualizarDias()"><span>${diasNombres[i]}</span>`;container.appendChild(div)}}function actualizarTiposHorarios(){const container=document.getElementById('tiposHorarios');container.innerHTML='';if(tiposHorarios.length===0){container.innerHTML='<div style="text-align: center; padding: 40px; color: #666;"><h3>📝 No hay tipos de horarios configurados</h3><p>Crea tu primer tipo de horario arriba para comenzar.</p></div>';return}tiposHorarios.forEach((tipo,index)=>{const div=document.createElement('div');div.className='tipo-horario';div.innerHTML=`<div class="tipo-header"><span class="tipo-nombre">📋 ${tipo.nombre}</span><label class="toggle-switch"><input type="checkbox" ${tipo.activo?'checked':''} onchange="toggleTipo(${index})"><span class="slider"></span></label></div><div class="form-group"><label>⏰ Agregar nuevo horario:</label><div class="input-group"><input type="time" id="horario${index}"><button class="btn btn-primary btn-small" onclick="agregarHorario(${index})">➕ AGREGAR</button></div></div><div class="horarios-lista">${tipo.horarios.length===0?'<p style="grid-column: 1/-1; text-align: center; color: #666; font-style: italic;">No hay horarios configurados</p>':tipo.horarios.map((horario,hIndex)=>`<div class="horario-item"><span>⏰ ${horario}</span><button class="btn btn-danger btn-small" onclick="eliminarHorario(${index}, ${hIndex})" title="Eliminar horario">❌</button></div>`).join('')}</div>`;container.appendChild(div)})}async function sonarManual(){try{const btn=event.target;btn.disabled=true;btn.innerHTML='🔔 SONANDO...';await fetch('/api/sonar',{method:'POST'});alert('🔔 Timbre activado manualmente');setTimeout(()=>{btn.disabled=false;btn.innerHTML='🔔 SONAR TIMBRE'},3000)}catch(error){alert('❌ Error al activar el timbre');event.target.disabled=false;event.target.innerHTML='🔔 SONAR TIMBRE'}}async function toggleSistema(){try{const response=await fetch('/api/toggle',{method:'POST'});const data=await response.json();sistemaActivo=data.sistemaActivo;alert(sistemaActivo?'✅ Sistema activado':'⚠️ Sistema desactivado');actualizarInterfaz()}catch(error){alert('❌ Error al cambiar estado del sistema')}}async function crearTipoHorario(){const nombre=document.getElementById('nuevoTipo').value.trim();if(!nombre){alert('⚠️ Por favor ingresa un nombre para el tipo de horario');return}try{await fetch('/api/configurar',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({accion:'agregar_tipo',nombre:nombre})});document.getElementById('nuevoTipo').value='';alert(`✅ Tipo "${nombre}" creado correctamente`);cargarEstado()}catch(error){alert('❌ Error al crear tipo de horario')}}async function agregarHorario(tipoIndex){const input=document.getElementById(`horario${tipoIndex}`);const horario=input.value;if(!horario){alert('⚠️ Por favor selecciona una hora');return}try{await fetch('/api/configurar',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({accion:'agregar_horario',tipoIndex:tipoIndex,horario:horario})});input.value='';alert(`✅ Horario ${horario} agregado correctamente`);cargarEstado()}catch(error){alert('❌ Error al agregar horario')}}async function eliminarHorario(tipoIndex,horarioIndex){if(!confirm('🗑️ ¿Estás seguro de eliminar este horario?'))return;try{await fetch('/api/configurar',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({accion:'eliminar_horario',tipoIndex:tipoIndex,horarioIndex:horarioIndex})});alert('🗑️ Horario eliminado correctamente');cargarEstado()}catch(error){alert('❌ Error al eliminar horario')}}async function toggleTipo(tipoIndex){try{await fetch('/api/configurar',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({accion:'toggle_tipo',tipoIndex:tipoIndex})});cargarEstado()}catch(error){alert('❌ Error al cambiar estado del tipo')}}async function guardarDias(){try{await fetch('/api/dias',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({dias:estadoDias})});alert('💾 Configuración de días guardada correctamente')}catch(error){alert('❌ Error al guardar configuración de días')}}
```

## 📁 Configuración LittleFS

### Detectar Partition Scheme

**En PowerShell:**

```powershell
# Buscar archivos de particiones
dir "C:\Users\[usuario]\AppData\Local\Arduino15\packages\esp32\hardware\esp32\*\tools\partitions\*.csv"

# Ver partición por defecto
Get-Content "C:\Users\[usuario]\AppData\Local\Arduino15\packages\esp32\hardware\esp32\*\tools\partitions\default.csv" | Select-String "spiffs"
```

### Configurar Variables

```powershell
# Variables del sistema
$MK = (Get-ChildItem "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools\mklittlefs\*\mklittlefs*.exe" | Select-Object -Last 1).FullName

$ESPTOOL = (Get-ChildItem "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools\esptool_py\*\esptool.exe" | Select-Object -Last 1).FullName
```

### Esquemas de Partición Comunes

#### Default 4MB with spiffs
```powershell
$OFFSET = "0x290000"
$SIZE = "0x170000"  # 1.5MB
```

#### No OTA (2MB APP/2MB SPIFFS)
```powershell
$OFFSET = "0x200000"
$SIZE = "0x200000"  # 2MB
```

#### Huge APP (3MB No OTA/1MB SPIFFS)
```powershell
$OFFSET = "0x300000"
$SIZE = "0x100000"  # 1MB
```

### Crear y Flashear LittleFS

```powershell
# Navegar a carpeta del proyecto
cd "C:\ruta\a\tu\proyecto\timbre"

# Verificar estructura
dir
dir data\

# Crear imagen LittleFS
& $MK -c .\data -b 4096 -p 256 -s $SIZE .\littlefs.bin

# Verificar imagen creada
dir littlefs.bin

# Flashear al ESP32
& $ESPTOOL --chip esp32 --port COM4 --baud 921600 write_flash $OFFSET .\littlefs.bin
```

## 🔧 Configuración Arduino IDE

### Configuración de la Placa

1. **Tools → Board → ESP32 Arduino → ESP32 Dev Module**
2. **Tools → Partition Scheme:** seleccionar según el esquema usado
3. **Tools → Port:** seleccionar puerto COM correcto
4. **Tools → Upload Speed:** 921600

### Verificar Configuración

```cpp
// En setup(), agregar para diagnóstico:
Serial.println("=== DIAGNÓSTICO LITTLEFS ===");
File root = LittleFS.open("/");
File file = root.openNextFile();
while (file) {
    Serial.printf("Archivo: %s (%u bytes)\n", file.name(), (unsigned)file.size());
    file = root.openNextFile();
}
Serial.println("=== FIN DIAGNÓSTICO ===");
```

## 📤 Subida del Código

### Proceso de Subida

1. **Conectar ESP32** al puerto USB
2. **Abrir Arduino IDE**
3. **Abrir proyecto** `timbre.ino`
4. **Verificar configuración** de placa y puerto
5. **Compilar:** Sketch → Verify/Compile
6. **Subir:** Sketch → Upload

### Verificar Funcionamiento

1. **Abrir Serial Monitor** (115200 baudios)
2. **Verificar logs** de inicio
3. **Buscar mensaje:** "Sistema listo!"
4. **Anotar IP:** generalmente `192.168.4.1`

## 🌐 Uso del Sistema

### Acceso Web

1. **Conectar a WiFi:** "Timbre 23 de Marzo"
2. **Contraseña:** "mariaelena13376222"
3. **Abrir navegador:** `http://192.168.4.1`

### Configurar Horarios

1. **Crear tipo de horario:** ej. "Recreos"
2. **Agregar horarios:** ej. 10:30, 12:00
3. **Activar/desactivar** tipos según necesidad
4. **Configurar días** de la semana activos

### Funciones Principales

- **Sonar timbre manual**
- **Activar/desactivar sistema**
- **Ver logs en tiempo real**
- **Configurar múltiples tipos de horarios**
- **Control por días de la semana**

## 🚨 Solución de Problemas

### Error: "Archivo no encontrado"

**Causa:** LittleFS vacío o mal configurado

**Solución:**
```powershell
# Verificar archivos en data/
dir data\

# Recrear imagen con offset correcto
Remove-Item "littlefs.bin" -ErrorAction SilentlyContinue
& $MK -c .\data -b 4096 -p 256 -s $SIZE .\littlefs.bin
& $ESPTOOL --chip esp32 --port COM4 --baud 921600 write_flash $OFFSET .\littlefs.bin
```

### Error: "assert failed: lfs_fs_grow_"

**Causa:** Desajuste entre tamaño de partición y imagen

**Solución:**
1. Verificar partition scheme en Arduino IDE
2. Usar OFFSET y SIZE correctos para ese esquema
3. Recrear imagen con tamaño exacto

### Error: WiFi no se conecta

**Causa:** Configuración de red

**Solución:**
1. Verificar nombre de red en código
2. Cambiar canal WiFi en código
3. Reiniciar ESP32

### Error: Puerto COM ocupado

**Causa:** Serial Monitor abierto durante flasheo

**Solución:**
1. Cerrar Serial Monitor
2. Desconectar y reconectar ESP32
3. Intentar flasheo nuevamente

### Error: OLED no funciona

**Causa:** Conexiones I2C o dirección incorrecta

**Solución:**
1. Verificar conexiones SDA/SCL
2. Verificar dirección I2C (0x3C vs 0x3D)
3. El sistema funciona sin OLED

## 📞 Contacto y Soporte

Para soporte técnico o mejoras:
- Verificar Serial Monitor para logs detallados
- Documentar errores específicos
- Incluir configuración de hardware usada

---

## 📝 Changelog

**v1.0** - Sistema básico con WiFi y horarios
**v1.1** - Agregada pantalla OLED y logs
**v1.2** - Interfaz web mejorada y LittleFS
**v1.3** - Soporte para múltiples partition schemes

---

**✅ ¡Sistema de Timbre Automático listo para usar!** 🔔

Remove-Item "littlefs.bin" -ErrorAction SilentlyContinue
$MK = "C:\Users\stack\AppData\Local\Arduino15\packages\esp32\tools\mklittlefs\3.0.0-gnu12-dc7f933\mklittlefs.exe"
$OFFSET = "0x210000"
$SIZE = "0x1E0000"
& $MK -c .\data -b 4096 -p 256 -s $SIZE .\littlefs.bin
& "C:\Users\stack\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.0.0\esptool.exe" --chip esp32 --port COM4 --baud 921600 write_flash $OFFSET .\littlefs.bin

