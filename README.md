# NetChecker

**Comprobador de conectividad TCP/UDP contra servidores empresariales**

Aplicación MFC SDI para Windows que verifica la accesibilidad de puertos TCP/UDP
contra Domain Controllers, Print Servers, SCCM Full y SCCM Distribution Points
de forma asíncrona, con informe HTML exportable.

Desarrollada con la asistencia de **Claude Sonnet** (Anthropic).  
Distribución libre — free to use, copy and modify.

---

## Requisitos de compilación

| Requisito | Valor |
|---|---|
| IDE | Visual Studio 2026 |
| Plataforma | x64 |
| Juego de caracteres | Unicode |
| MFC | Dinámico (`_AFXDLL`) |
| Estándar C++ | C++20 (`/std:c++20`) |
| Librerías extra | `ws2_32.lib`, `iphlpapi.lib` |
| PCH | `src/pch.h` |

## Compilar

```
1. Abrir NetChecker.sln en Visual Studio 2026
2. Seleccionar Debug|x64 o Release|x64
3. Build → Build Solution  (Ctrl+Shift+B)
```

El ejecutable se genera en `x64\Debug\NetChecker.exe` o `x64\Release\NetChecker.exe`.

---

## Arquitectura

```
NetChecker.sln
└── NetChecker.vcxproj
    └── src/
        ├── pch.h / pch.cpp              ← Cabecera precompilada (Winsock2 primero)
        ├── resource.h                   ← IDs de recursos
        ├── NetChecker.rc                ← Diálogos, strings, iconos, manifest
        ├── NetChecker.manifest          ← ComCtl v6 / DPI awareness
        │
        ├── AppTypes.h                   ← Tipos compartidos (sin MFC)
        ├── PortDB.cpp                   ← Base de datos de puertos por tipo de servidor
        │
        ├── XmlLite.h/.cpp               ← Parser/writer XML propio (sin COM/MSXML)
        ├── ConfigManager.h/.cpp         ← Lee/escribe NetChecker.config (XML)
        ├── NetworkChecker.h/.cpp        ← Checks TCP/UDP asíncronos (std::thread)
        ├── HtmlExporter.h/.cpp          ← Informe HTML con resultados
        │
        ├── NetChecker.h/.cpp            ← CNetCheckerApp : CWinApp
        ├── MainFrame.h/.cpp             ← CMainFrame : CFrameWnd (SDI)
        ├── ResultListCtrl.h/.cpp        ← CResultListCtrl : CListCtrl (subclased)
        ├── ConfigEditorDlg.h/.cpp       ← Editor de configuración con pestañas
        └── AboutDlg.h/.cpp              ← Diálogo Acerca de
```

### Módulos principales

| Clase / namespace | Responsabilidad |
|---|---|
| `PortDB` | Devuelve la lista de `PortEntry` por `DestinationType` |
| `StrUtil` | Convierte enums (`ConnectStatus`, `Protocol`) a texto |
| `XmlWriter` / `XmlParse` | Serialización XML sin librerías externas |
| `ConfigManager` | Lee/escribe `NetChecker.config` |
| `NetworkChecker` | Checks TCP (connect no bloqueante + select) y UDP (send+recv+SIO_UDP_CONNRESET) |
| `HtmlExporter` | Genera informe HTML autocontenido con tabla por servidor |
| `CConfigEditorDlg` | Editor visual de servidores y puertos con pestañas y cuadrícula ordenable |

### Barra de herramientas

Iconos ICO renderizados en `CImageList` (modo activo y deshabilitado con manipulación
directa de píxeles DIB, sin GDI+). El botón de información se posiciona en el extremo
derecho mediante un separador de anchura dinámica calculada en `OnSize`.

| Botón | Acción |
|---|---|
| Comprobar / Detener | Lanza o cancela la verificación (toggle) |
| Guardar HTML | Exporta informe a `NetChecker_AAAAMMDD_HHMM.html` |
| Guardar config | Persiste la configuración actual en disco |
| Recargar config | Recarga el archivo de configuración desde disco |
| Editor de configuración | Abre el editor de servidores y puertos |
| Salir | Cierra la aplicación |
| Información | Muestra el diálogo Acerca de (extremo derecho) |

### Editor de configuración (CConfigEditorDlg)

- Formulario para añadir servidores: nombre, IP (control `SysIPAddress32`), tipo.
- Al agregar un servidor se crea una pestaña con la cuadrícula de puertos TCP/UDP
  por defecto para ese tipo.
- La cuadrícula permite añadir, modificar y eliminar puertos individualmente.
- Clic en cabecera **Puerto** o **Protocolo** para ordenar ascendente/descendente
  (flecha ▲/▼ en la cabecera; orden secundario por número de puerto).
- Al pulsar Aceptar los cambios se guardan en `NetChecker.config`.
- El mismo editor sirve para crear una configuración nueva y para editar la existente.

### Comprobación de conectividad

**TCP**: `connect()` no bloqueante + `select()` con timeout de 2 s.

**UDP**: `send()` + `recv()` con `SIO_UDP_CONNRESET` habilitado para recibir errores ICMP.

| Resultado UDP | Estado mostrado |
|---|---|
| Datos recibidos | **OK** |
| ICMP port-unreachable (`WSAECONNRESET`) | **CERRADO** |
| Timeout sin respuesta | **SIN RESPUESTA** *(abierto o filtrado)* |

Los resultados se envían al hilo principal mediante `PostMessage`
(`WM_NC_RESULT` / `WM_NC_COMPLETE`).

### ListView — CResultListCtrl

- Grupos por servidor (`EnableGroupView`); dentro de cada grupo, TCP primero y luego UDP.
- Checkbox por fila para activar/desactivar puertos individuales (dibujado manualmente).
- Menú contextual (botón derecho): habilitar/deshabilitar todos, solo TCP o solo UDP.
- Color de texto en columna Estado via `NM_CUSTOMDRAW`.

### Flujo de configuración

```
Arranque
 ├─ Existe NetChecker.config → Cargar → Poblar ListView
 └─ No existe               → Abrir CConfigEditorDlg → Guardar → Poblar

Edición en lista → m_cfgDirty = true
"Guardar config" → DoSaveCfg() → disco → m_cfgDirty = false
"Recargar"       → DoReloadConfig(force=true)
WM_CLOSE         → si m_cfgDirty → Sí / No / Cancelar
```

### Formato del archivo de configuración (XML)

La IP de origen **no** se persiste; se detecta automáticamente en tiempo de ejecución.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<NetCheckerConfig version="3">
  <Destinations>
    <Destination name="DC01" ip="10.0.0.1" type="DC">
      <Ports>
        <Port port="88"  protocol="TCP" description="Kerberos" enabled="1"/>
        <Port port="389" protocol="TCP" description="LDAP"     enabled="1"/>
      </Ports>
    </Destination>
  </Destinations>
</NetCheckerConfig>
```

---

## Puertos por defecto por tipo de servidor

### Domain Controller (DC)
53 TCP/UDP DNS · 88 TCP/UDP Kerberos · 135 TCP RPC Endpoint Mapper ·
139 TCP NetBIOS · 389 TCP/UDP LDAP · 445 TCP SMB · 464 TCP/UDP Kerberos Pwd ·
636 TCP LDAPS · 3268 TCP Global Catalog · 3269 TCP GC SSL · 9389 TCP AD Web Services

### Print Server
80 TCP HTTP · 443 TCP HTTPS · 445 TCP SMB · 515 TCP LPD · 631 TCP IPP · 9100 TCP RAW

### SCCM Full
80 TCP HTTP · 443 TCP HTTPS · 445 TCP SMB · 135 TCP RPC ·
8530 TCP WSUS HTTP · 8531 TCP WSUS HTTPS · 2701 TCP Remote Control · 10123 TCP MP alt

### SCCM Distribution Point
80 TCP HTTP · 443 TCP HTTPS · 445 TCP SMB · 8530 TCP WSUS HTTP · 8531 TCP WSUS HTTPS

---

## Informe HTML

El informe se genera con nombre `NetChecker_AAAAMMDD_HHMM.html`. Incluye:
- IP de origen en la cabecera junto al título.
- Una tabla por servidor con columnas de ancho fijo consistentes entre servidores.
- Puertos TCP primero, luego UDP dentro de cada tabla.
- Colores de estado: verde (OK), rojo (CERRADO), azul-gris (SIN RESPUESTA),
  ámbar (DESCONOCIDO), gris (deshabilitado).

---

## Iconos

Los iconos de la barra de herramientas han sido obtenidos de:

**[Icons8 — Fluency style](https://icons8.com/icons/fluency)**

| Archivo | Uso en la aplicación |
|---|---|
| icon_run.ico | Comprobar conectividad |
| icon_stop.ico | Detener comprobación |
| icon_html.ico | Guardar informe HTML |
| icon_save.ico | Guardar configuración |
| icon_reload.ico | Recargar configuración |
| icon_cfgedit.ico | Editor de configuración |
| icon_info.ico | Acerca de / Información |
| icon_exit.ico | Salir |

---

## Licencia

Software de libre distribución. Sin restricciones de uso, copia o modificación.
