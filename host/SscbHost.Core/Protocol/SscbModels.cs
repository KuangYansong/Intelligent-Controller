namespace SscbHost.Core.Protocol;

public sealed record RealtimeData(
    double VoltageV,
    double CurrentA,
    double TemperatureC,
    SscbStatusBits Status);

public sealed record FaultData(
    SscbFaultCode Fault,
    uint TimestampMs,
    double CurrentA,
    double VoltageLite);

public sealed record HeartbeatData(
    byte NodeId,
    SscbSystemState State,
    ulong RuntimeSeconds);

public sealed record ParamResponse(
    bool Ok,
    SscbParamId ParamId,
    float Value);

[Flags]
public enum SscbStatusBits : byte
{
    None = 0,
    Normal = 1 << 0,
    ShortFault = 1 << 1,
    OverloadFault = 1 << 2,
    OvervoltageFault = 1 << 3,
    OvertempFault = 1 << 4,
    Lockout = 1 << 5,
    SelftestFailed = 1 << 6
}
