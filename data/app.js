// Variables globales
let sistemaActivo = true;
let estadoDias = [true, true, true, true, true, true, true];
let tiposHorarios = [];
const diasNombres = ['Lunes', 'Martes', 'Miércoles', 'Jueves', 'Viernes', 'Sábado', 'Domingo'];

// Inicialización
window.onload = function() {
    console.log('🚀 Aplicación iniciada');
    actualizarHora();
    cargarEstado();
    
    // Actualizar hora cada segundo
    setInterval(actualizarHora, 1000);
    
    // Cargar estado cada 5 segundos
    setInterval(cargarEstado, 5000);
    
    // Cargar logs cada 2 segundos
    setInterval(cargarLogs, 2000);
};

// Función para actualizar la hora
async function actualizarHora() {
    try {
        const response = await fetch('/api/hora');
        const data = await response.json();
        document.getElementById('horaActual').innerHTML = 
            `📅 ${data.fecha} ⏰ ${data.hora}`;
    } catch (error) {
        console.error('❌ Error al obtener hora:', error);
        document.getElementById('horaActual').innerHTML = '❌ Error de conexión';
    }
}

// Función para cargar el estado del sistema
async function cargarEstado() {
    try {
        const response = await fetch('/api/estado');
        const data = await response.json();
        
        sistemaActivo = data.sistemaActivo;
        estadoDias = data.dias;
        tiposHorarios = data.tipos;
        
        actualizarInterfaz();
    } catch (error) {
        console.error('❌ Error al cargar estado:', error);
    }
}

// Función para cargar logs
async function cargarLogs() {
    try {
        const response = await fetch('/api/logs');
        const data = await response.json();
        
        if (data.ultimoLog) {
            const timestamp = new Date(data.timestamp).toLocaleTimeString();
            document.getElementById('logDisplay').innerHTML = 
                `[${timestamp}] ${data.ultimoLog}`;
        }
    } catch (error) {
        console.error('❌ Error al cargar logs:', error);
    }
}

// Función para actualizar la interfaz
function actualizarInterfaz() {
    // Actualizar estado del sistema
    const estadoEl = document.getElementById('estadoSistema');
    const btnToggle = document.getElementById('btnToggle');
    
    if (sistemaActivo) {
        estadoEl.textContent = '✅ Sistema Activo - Funcionando Correctamente';
        estadoEl.className = 'estado-sistema sistema-activo';
        btnToggle.innerHTML = '❌ DESACTIVAR SISTEMA';
        btnToggle.className = 'btn btn-danger';
    } else {
        estadoEl.textContent = '⚠️ Sistema Inactivo - Timbre Deshabilitado';
        estadoEl.className = 'estado-sistema sistema-inactivo';
        btnToggle.innerHTML = '✅ ACTIVAR SISTEMA';
        btnToggle.className = 'btn btn-success';
    }
    
    // Actualizar días
    actualizarDias();
    
    // Actualizar tipos de horarios
    actualizarTiposHorarios();
}

// Función para actualizar los días de la semana
function actualizarDias() {
    const container = document.getElementById('diasSemana');
    container.innerHTML = '';
    
    for (let i = 0; i < 7; i++) {
        const div = document.createElement('div');
        div.className = `dia-checkbox ${estadoDias[i] ? 'dia-activo' : ''}`;
        div.innerHTML = `
            <input type="checkbox" ${estadoDias[i] ? 'checked' : ''} 
                   onchange="estadoDias[${i}] = this.checked; actualizarDias()">
            <span>${diasNombres[i]}</span>
        `;
        container.appendChild(div);
    }
}

// Función para actualizar tipos de horarios
function actualizarTiposHorarios() {
    const container = document.getElementById('tiposHorarios');
    container.innerHTML = '';
    
    if (tiposHorarios.length === 0) {
        container.innerHTML = `
            <div style="text-align: center; padding: 40px; color: #666;">
                <h3>📝 No hay tipos de horarios configurados</h3>
                <p>Crea tu primer tipo de horario arriba para comenzar.</p>
            </div>
        `;
        return;
    }
    
    tiposHorarios.forEach((tipo, index) => {
        const div = document.createElement('div');
        div.className = 'tipo-horario';
        div.innerHTML = `
            <div class="tipo-header">
                <span class="tipo-nombre">📋 ${tipo.nombre}</span>
                <label class="toggle-switch">
                    <input type="checkbox" ${tipo.activo ? 'checked' : ''} 
                           onchange="toggleTipo(${index})">
                    <span class="slider"></span>
                </label>
            </div>
            
            <div class="form-group">
                <label>⏰ Agregar nuevo horario:</label>
                <div class="input-group">
                    <input type="time" id="horario${index}">
                    <button class="btn btn-primary btn-small" onclick="agregarHorario(${index})">
                        ➕ AGREGAR
                    </button>
                </div>
            </div>
            
            <div class="horarios-lista">
                ${tipo.horarios.length === 0 ? 
                    '<p style="grid-column: 1/-1; text-align: center; color: #666; font-style: italic;">No hay horarios configurados</p>' :
                    tipo.horarios.map((horario, hIndex) => `
                        <div class="horario-item">
                            <span>⏰ ${horario}</span>
                            <button class="btn btn-danger btn-small" onclick="eliminarHorario(${index}, ${hIndex})" title="Eliminar horario">
                                ❌
                            </button>
                        </div>
                    `).join('')
                }
            </div>
        `;
        container.appendChild(div);
    });
}

// Función para sonar el timbre manualmente
async function sonarManual() {
    try {
        const btn = event.target;
        btn.disabled = true;
        btn.innerHTML = '🔔 SONANDO...';
        
        await fetch('/api/sonar', { method: 'POST' });
        
        // Mostrar confirmación
        mostrarNotificacion('🔔 Timbre activado manualmente', 'success');
        
        setTimeout(() => {
            btn.disabled = false;
            btn.innerHTML = '🔔 SONAR TIMBRE';
        }, 3000);
        
    } catch (error) {
        console.error('❌ Error al sonar timbre:', error);
        mostrarNotificacion('❌ Error al activar el timbre', 'error');
        event.target.disabled = false;
        event.target.innerHTML = '🔔 SONAR TIMBRE';
    }
}

// Función para toggle del sistema
async function toggleSistema() {
    try {
        const response = await fetch('/api/toggle', { method: 'POST' });
        const data = await response.json();
        sistemaActivo = data.sistemaActivo;
        
        mostrarNotificacion(
            sistemaActivo ? '✅ Sistema activado' : '⚠️ Sistema desactivado', 
            sistemaActivo ? 'success' : 'warning'
        );
        
        actualizarInterfaz();
    } catch (error) {
        console.error('❌ Error al cambiar estado del sistema:', error);
        mostrarNotificacion('❌ Error al cambiar estado del sistema', 'error');
    }
}

// Función para crear tipo de horario
async function crearTipoHorario() {
    const nombre = document.getElementById('nuevoTipo').value.trim();
    if (!nombre) {
        mostrarNotificacion('⚠️ Por favor ingresa un nombre para el tipo de horario', 'warning');
        return;
    }
    
    try {
        await fetch('/api/configurar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                accion: 'agregar_tipo',
                nombre: nombre
            })
        });
        
        document.getElementById('nuevoTipo').value = '';
        mostrarNotificacion(`✅ Tipo "${nombre}" creado correctamente`, 'success');
        cargarEstado();
    } catch (error) {
        console.error('❌ Error al crear tipo de horario:', error);
        mostrarNotificacion('❌ Error al crear tipo de horario', 'error');
    }
}

// Función para agregar horario
async function agregarHorario(tipoIndex) {
    const input = document.getElementById(`horario${tipoIndex}`);
    const horario = input.value;
    
    if (!horario) {
        mostrarNotificacion('⚠️ Por favor selecciona una hora', 'warning');
        return;
    }
    
    try {
        await fetch('/api/configurar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                accion: 'agregar_horario',
                tipoIndex: tipoIndex,
                horario: horario
            })
        });
        
        input.value = '';
        mostrarNotificacion(`✅ Horario ${horario} agregado correctamente`, 'success');
        cargarEstado();
    } catch (error) {
        console.error('❌ Error al agregar horario:', error);
        mostrarNotificacion('❌ Error al agregar horario', 'error');
    }
}

// Función para eliminar horario
async function eliminarHorario(tipoIndex, horarioIndex) {
    if (!confirm('🗑️ ¿Estás seguro de eliminar este horario?')) return;
    
    try {
        await fetch('/api/configurar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                accion: 'eliminar_horario',
                tipoIndex: tipoIndex,
                horarioIndex: horarioIndex
            })
        });
        
        mostrarNotificacion('🗑️ Horario eliminado correctamente', 'success');
        cargarEstado();
    } catch (error) {
        console.error('❌ Error al eliminar horario:', error);
        mostrarNotificacion('❌ Error al eliminar horario', 'error');
    }
}

// Función para toggle tipo de horario
async function toggleTipo(tipoIndex) {
    try {
        await fetch('/api/configurar', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                accion: 'toggle_tipo',
                tipoIndex: tipoIndex
            })
        });
        
        cargarEstado();
    } catch (error) {
        console.error('❌ Error al cambiar estado del tipo:', error);
        mostrarNotificacion('❌ Error al cambiar estado del tipo', 'error');
    }
}

// Función para guardar días
async function guardarDias() {
    try {
        await fetch('/api/dias', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                dias: estadoDias
            })
        });
        
        mostrarNotificacion('💾 Configuración de días guardada correctamente', 'success');
    } catch (error) {
        console.error('❌ Error al guardar días:', error);
        mostrarNotificacion('❌ Error al guardar configuración de días', 'error');
    }
}

// Función para mostrar notificaciones
function mostrarNotificacion(mensaje, tipo = 'info') {
    // Crear elemento de notificación
    const notificacion = document.createElement('div');
    notificacion.className = `notificacion notificacion-${tipo}`;
    notificacion.innerHTML = `
        <div class="notificacion-content">
            ${mensaje}
            <button onclick="this.parentElement.parentElement.remove()" class="notificacion-close">×</button>
        </div>
    `;
    
    // Agregar estilos si no existen
    if (!document.getElementById('notificacion-styles')) {
        const styles = document.createElement('style');
        styles.id = 'notificacion-styles';
        styles.innerHTML = `
            .notificacion {
                position: fixed;
                top: 20px;
                right: 20px;
                padding: 15px 20px;
                border-radius: 10px;
                color: white;
                font-weight: bold;
                z-index: 1000;
                animation: slideIn 0.3s ease;
                max-width: 400px;
                box-shadow: 0 5px 15px rgba(0,0,0,0.3);
            }
            
            .notificacion-content {
                display: flex;
                justify-content: space-between;
                align-items: center;
                gap: 10px;
            }
            
            .notificacion-close {
                background: none;
                border: none;
                color: white;
                font-size: 1.5em;
                cursor: pointer;
                padding: 0;
                width: 30px;
                height: 30px;
                border-radius: 50%;
                display: flex;
                align-items: center;
                justify-content: center;
            }
            
            .notificacion-close:hover {
                background: rgba(255,255,255,0.2);
            }
            
            .notificacion-success {
                background: linear-gradient(45deg, #28a745, #20c997);
            }
            
            .notificacion-error {
                background: linear-gradient(45deg, #dc3545, #fd7e14);
            }
            
            .notificacion-warning {
                background: linear-gradient(45deg, #ffc107, #fd7e14);
                color: #212529;
            }
            
            .notificacion-info {
                background: linear-gradient(45deg, #17a2b8, #6f42c1);
            }
            
            @keyframes slideIn {
                from {
                    transform: translateX(100%);
                    opacity: 0;
                }
                to {
                    transform: translateX(0);
                    opacity: 1;
                }
            }
        `;
        document.head.appendChild(styles);
    }
    
    // Agregar al DOM
    document.body.appendChild(notificacion);
    
    // Auto-eliminar después de 5 segundos
    setTimeout(() => {
        if (notificacion.parentElement) {
            notificacion.remove();
        }
    }, 5000);
}

// Manejar errores globales
window.addEventListener('error', function(event) {
    console.error('❌ Error global:', event.error);
});

// Manejar promesas rechazadas
window.addEventListener('unhandledrejection', function(event) {
    console.error('❌ Promesa rechazada:', event.reason);
});

console.log('📱 App.js cargado correctamente');