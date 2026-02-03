# Richard-yq Feature Development Documentation

## Overview

This document provides comprehensive documentation of features developed by Richard-yq for the OAI O1-Adapter project. The development focuses on enhancing security monitoring capabilities and ensuring compliance with 3GPP standards.

**Development Branch:** `prach-alarms`  
**Total Commits:** 8  
**Primary Focus:** PRACH Attack Detection and Standards Compliance

---

## Table of Contents

1. [Feature 1: PRACH Attack Detection Alarm Support](#feature-1-prach-attack-detection-alarm-support)
2. [Feature 2: VES Schema Reference Standardization](#feature-2-ves-schema-reference-standardization)
3. [Feature 3: OAI gNB Configuration Integration](#feature-3-oai-gnb-configuration-integration)
4. [Technical Implementation Details](#technical-implementation-details)
5. [Configuration Changes](#configuration-changes)
6. [Impact and Benefits](#impact-and-benefits)

---

## Feature 1: PRACH Attack Detection Alarm Support

**Commit:** `56acba3` - "add PRACH attack detection alarm support"

### Background

PRACH (Physical Random Access Channel) is a critical component in 5G/LTE networks used for initial access procedures. PRACH attacks can cause:
- Network resource exhaustion
- Denial of Service (DoS) to legitimate users
- Performance degradation of the base station
- Potential security breaches

### Implementation

This feature introduces a new security alarm mechanism to detect and report PRACH-based attacks in real-time through the O1 interface.

#### Alarm Specification

- **Alarm Name:** `prachAttackDetected`
- **Severity:** `CRITICAL`
- **Type:** `SECURITY_SERVICE_OR_MECHANISM_VIOLATION`
- **Notification Method:** VES (Virtual Event Streaming) to SMO
- **NETCONF Integration:** Full integration with sysrepo datastore

#### Key Components Added

1. **Alarm Structure** (in `src/alarms/alarms.c`):
```c
static alarm_t alarm_prach_attack_detected = {
    .alarm = "prachAttackDetected",
    .severity = ALARM_SEVERITY_CRITICAL,
    .type = ALARM_TYPE_SECURITY_SERVICE_OR_MECHANISM_VIOLATION,
    .object_instance = 0,
    .state = ALARM_STATE_CLEARED,
    .timeout = 0,
};
```

2. **Trigger Function** (in `src/alarms/alarms.c`):
```c
int alarms_prach_attack_trigger() {
    if(alarm_prach_attack_detected.state == ALARM_STATE_CLEARED) {
        alarm_prach_attack_detected.state = ALARM_STATE_RAISE;
        alarm_prach_attack_detected.timeout = 0;  // Immediate raise
    }
    else if(alarm_prach_attack_detected.state == ALARM_STATE_CLEAR) {
        // Cancel clear operation, keep alarm raised
        alarm_prach_attack_detected.state = ALARM_STATE_RAISED;
    }
    return 0;
}
```

3. **API Declaration** (in `src/alarms/alarms.h`):
```c
int alarms_prach_attack_trigger();
```

#### Workflow

1. **Detection Phase:**
   - OAI gNB detects PRACH attack through its internal monitoring
   - Detection information available via telnet interface

2. **Notification Phase:**
   - Adapter calls `alarms_prach_attack_trigger()`
   - Alarm state transitions to `ALARM_STATE_RAISE`
   - Immediate trigger (timeout = 0) for critical security events

3. **VES Reporting Phase:**
   - Alarm serialized to VES format using template
   - Sent to SMO VES Collector via HTTP/HTTPS
   - Includes full context: node-id, timestamp, severity, type

4. **NETCONF Update Phase:**
   - Alarm synchronized to sysrepo datastore
   - Available for NETCONF queries
   - Notification sent to subscribed NETCONF clients

#### VES Message Format

The alarm generates a VES `notifyNewAlarm` message:
```json
{
    "event": {
        "commonEventHeader": {
            "domain": "fault",
            "eventType": "stndDefined_Alarms",
            "priority": "HIGH",
            "stndDefinedNamespace": "3GPP-FaultSupervision"
        },
        "stndDefinedFields": {
            "schemaReference": "https://forge.3gpp.org/rep/sa5/MnS/blob/SA88-Rel16/OpenAPI/faultMnS.yaml",
            "data": {
                "notificationType": "notifyNewAlarm",
                "alarmType": "SECURITY_SERVICE_OR_MECHANISM_VIOLATION",
                "probableCause": "prachAttackDetected",
                "perceivedSeverity": "CRITICAL"
            }
        }
    }
}
```

---

## Feature 2: VES Schema Reference Standardization

**Commits:**
- `3864028` - "IMPORTANT: Change schemaReference URL to another 3GPP file"
- `6418874` - "IMPORTANT: Update VES-xxx.json schemaReference to 3gpp URL"

### Purpose

Ensure full compliance with 3GPP TS 28.532 specifications by using official 3GPP schema references in VES messages.

### Changes Made

Updated `schemaReference` field in all VES templates to point to official 3GPP repositories:

**Before:**
```json
"schemaReference": "https://www.3gpp.org/ftp/Specs/archive/28_series/28.532/schema/..."
```

**After:**
```json
"schemaReference": "https://forge.3gpp.org/rep/sa5/MnS/blob/SA88-Rel16/OpenAPI/faultMnS.yaml"
```

### Files Updated

1. **ves-new-alarm.json** - New alarm notifications
2. **ves-clear-alarm.json** - Alarm clearing notifications  
3. **ves-file-ready.json** - PM data file ready notifications

### Benefits

- ✅ Standards compliant with 3GPP Release 16
- ✅ Compatible with O-RAN SC SMO
- ✅ Compatible with ONAP SMO implementations
- ✅ Future-proof for standards evolution
- ✅ Proper schema validation at SMO side

### Technical Details

The schemaReference field is used by:
- **VES Collector:** To validate incoming messages
- **SMO:** To understand message structure and content
- **Analytics:** For proper data processing and correlation

Using official 3GPP forge repository ensures:
- Always up-to-date schema definitions
- Version control and traceability
- Community-maintained accuracy

---

## Feature 3: OAI gNB Configuration Integration

**Commits:**
- `17f24f2` - "Modify config.json to match my OAI gNB config - host ip - port - ves url"
- `79032d4` - "Revise VES(SMO) URL"
- `b14b384` - "Revise host IP to match my OAI gNB IP"

### Purpose

Configure the O1 Adapter to properly integrate with a specific OAI gNB deployment environment.

### Configuration Changes

#### 1. Network Configuration
```json
{
    "network": {
        "host": "192.168.8.59",        // Updated to match gNB host
        "username": "netconf",
        "password": "netconf!",
        "netconf-port": 1830,
        "sftp-port": 1222
    }
}
```

**Purpose:** Configure adapter's external network identity for pnfRegistration and PM data file transfers.

#### 2. VES Collector Configuration
```json
{
    "ves": {
        "url": "http://192.168.8.69:30417/eventListener/v7",  // Updated SMO endpoint
        "username": "sample1",
        "password": "sample1"
    }
}
```

**Purpose:** Direct all VES messages (alarms, heartbeats, PM notifications) to the correct SMO VES Collector.

#### 3. Telnet Server Configuration
```json
{
    "telnet": {
        "host": "192.168.8.59",        // Updated to gNB telnet server
        "port": 9090
    }
}
```

**Purpose:** Connect to OAI gNB's telnet interface for:
- Real-time monitoring data
- Configuration changes
- Alarm trigger detection

### Network Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      SMO (192.168.8.69)                     │
│  ┌──────────────────┐         ┌──────────────────┐        │
│  │  VES Collector   │         │  NETCONF Client  │        │
│  │  :30417          │         │                  │        │
│  └─────────┬────────┘         └──────────┬───────┘        │
└────────────┼───────────────────────────────┼───────────────┘
             │                               │
             │ VES/HTTP                      │ NETCONF/SSH
             │                               │
┌────────────▼───────────────────────────────▼───────────────┐
│              O1 Adapter (192.168.8.59)                     │
│  ┌──────────────────┐         ┌──────────────────┐        │
│  │  VES Client      │         │  NETCONF Server  │        │
│  │                  │         │  :1830           │        │
│  └──────────────────┘         └──────────────────┘        │
│                                                             │
│  ┌──────────────────┐         ┌──────────────────┐        │
│  │  Telnet Client   │         │  SFTP Server     │        │
│  │                  │         │  :1222           │        │
│  └─────────┬────────┘         └──────────────────┘        │
└────────────┼────────────────────────────────────────────────┘
             │
             │ Telnet
             │
┌────────────▼───────────────────────────────────────────────┐
│              OAI gNB (192.168.8.59)                        │
│  ┌──────────────────────────────────────────────┐         │
│  │  Telnet Server :9090                         │         │
│  │  - Monitoring data                           │         │
│  │  - Configuration interface                   │         │
│  │  - PRACH attack detection                    │         │
│  └──────────────────────────────────────────────┘         │
└────────────────────────────────────────────────────────────┘
```

---

## Technical Implementation Details

### Alarm State Machine

The PRACH attack alarm follows a state machine pattern:

```
    CLEARED ──────────────┐
       │                  │
       │ Attack Detected  │ Attack Resolved (manual/automatic)
       │                  │
       ▼                  │
    RAISE ────────────────┤
       │                  │
       │ timeout=0        │
       │                  │
       ▼                  │
    RAISED ───────────────┘
```

**States:**
- `ALARM_STATE_CLEARED`: No attack detected, normal operation
- `ALARM_STATE_RAISE`: Pending alarm raising (immediate for PRACH)
- `ALARM_STATE_RAISED`: Active alarm, reported to SMO
- `ALARM_STATE_CLEAR`: Pending alarm clearing

### Integration Points

#### 1. Telnet Interface Integration
The adapter connects to OAI gNB via telnet to:
- Monitor PRACH attack detection events
- Retrieve real-time performance metrics
- Execute configuration changes

#### 2. VES Integration
All alarms are reported via VES:
- **Protocol:** HTTP/HTTPS
- **Format:** JSON (VES 7.x compatible)
- **Authentication:** Basic Auth
- **Endpoint:** Configurable SMO VES Collector

#### 3. NETCONF Integration
Alarms are synchronized to NETCONF:
- **Datastore:** sysrepo
- **Protocol:** SSH (port 1830)
- **Models:** 3GPP YANG models
- **Operations:** get, get-config, edit-config, notifications

### Code Structure

```
src/
├── alarms/
│   ├── alarms.c          # Alarm logic including PRACH detection
│   └── alarms.h          # Alarm API and structures
├── oai/
│   ├── oai.c             # OAI gNB telnet communication
│   └── oai_data.c        # Data parsing from telnet
├── ves/
│   ├── ves.c             # VES message generation and sending
│   └── ves_internal.c    # VES template processing
├── netconf/
│   ├── netconf_data.c    # NETCONF datastore operations
│   └── netconf_session.c # NETCONF server management
└── main.c                # Main event loop coordination
```

---

## Configuration Changes

### Complete config.json Example

```json
{
    "log-level": 3,
    "software-version": "1.0.0",
    
    "network": {
        "host": "192.168.8.59",
        "username": "netconf",
        "password": "netconf!",
        "netconf-port": 1830,
        "sftp-port": 1222
    },

    "ves": {
        "template": {
            "new-alarm": "/adapter/config/ves-new-alarm.json",
            "clear-alarm": "/adapter/config/ves-clear-alarm.json",
            "pnf-registration": "/adapter/config/ves-pnf-registration.json",
            "file-ready": "/adapter/config/ves-file-ready.json",
            "heartbeat": "/adapter/config/ves-heartbeat.json",
            "pm-data": "/adapter/config/pmData-measData.xml"
        },
        "pnf-registration": true,
        "heartbeat-interval": 30,
        "url": "http://192.168.8.69:30417/eventListener/v7",
        "username": "sample1",
        "password": "sample1",
        "file-expiry": 86400,
        "pm-data-interval": 30
    },

    "alarms": {
        "internal-connection-lost-timeout": 3,
        "load-downlink-exceeded-warning-threshold": 50,
        "load-downlink-exceeded-warning-timeout": 30
    },

    "telnet": {
        "host": "192.168.8.59",
        "port": 9090
    },

    "info": {
        "gnb-du-id": 0,
        "cell-local-id": 0,
        "node-id": "gNB-Eurecom-5GNRBox-00001",
        "location-name": "Test Lab Environment",
        "managed-by": "ManagementSystem=O-RAN-SC-ONAP-based-SMO",
        "managed-element-type": "NodeB",
        "model": "nr-softmodem",
        "unit-type": "gNB"
    }
}
```

---

## Impact and Benefits

### Security Enhancement

✅ **Real-time Attack Detection**
- Immediate notification of PRACH attacks
- Critical severity ensures high-priority handling
- Enables rapid incident response

✅ **Integration with Security Operations**
- VES messages can feed into SIEM systems
- NETCONF notifications for automated response
- Full audit trail via datastore persistence

### Standards Compliance

✅ **3GPP Compliance**
- Official schema references from 3GPP forge
- Alignment with TS 28.532 specifications
- Compatible with Release 16 standards

✅ **O-RAN Compatibility**
- Works with O-RAN SC SMO
- Compatible with ONAP-based deployments
- Follows O1 interface specifications

### Operational Benefits

✅ **Centralized Management**
- All security events visible in SMO
- Unified alarm management
- Coordinated with existing alarms (connection loss, load exceeded)

✅ **Deployment Flexibility**
- Configurable endpoints for different environments
- Easy adaptation to network topology
- Support for various SMO implementations

### Testing and Validation

The implementation has been validated in:
- **Environment:** OAI gNB with USRP hardware
- **Network:** 192.168.8.x test network
- **SMO:** VES Collector at 192.168.8.69:30417
- **Attack Scenarios:** PRACH flooding and anomaly detection

---

## Commit History

| Commit | Date | Description |
|--------|------|-------------|
| 6418874 | Latest | IMPORTANT: Update VES-xxx.json schemaReference to 3gpp URL |
| 3864028 | - | IMPORTANT: Change schemaReference URL to another 3GPP file |
| b14b384 | - | Revise host IP to match my OAI gNB IP |
| 79032d4 | - | Revise VES(SMO) URL |
| 17f24f2 | - | Modify config.json to match my OAI gNB config |
| 4696e06 | - | WIP on prach-alarms: add PRACH attack detection |
| 8869b87 | - | index on prach-alarms: add PRACH attack detection |
| 56acba3 | Earliest | add PRACH attack detection alarm support |

---

## Future Enhancements

Potential areas for future development:
1. **Advanced Attack Classification** - Distinguish different PRACH attack types
2. **Automatic Mitigation** - Trigger countermeasures via NETCONF edit operations
3. **Attack Analytics** - Historical data and pattern analysis
4. **Multi-Cell Correlation** - Coordinate attack detection across cells
5. **Machine Learning Integration** - AI-based anomaly detection

---

## References

- 3GPP TS 28.532: Management and orchestration; Generic management services
- 3GPP TS 38.321: NR; Medium Access Control (MAC) protocol specification (PRACH)
- O-RAN WG4: O1 Interface Specification
- ONAP VES 7.x: Virtual Event Streaming specification
- RFC 6241: Network Configuration Protocol (NETCONF)

---

## Contact

For questions or further information about this development:
- **Developer:** richard-yq
- **Branch:** prach-alarms
- **Repository:** o1-adapter
