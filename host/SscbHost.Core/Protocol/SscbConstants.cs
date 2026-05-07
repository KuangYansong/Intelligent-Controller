namespace SscbHost.Core.Protocol;

public static class SscbConstants
{
    public const ushort RealtimeBaseId = 0x100;
    public const ushort FaultBaseId = 0x200;
    public const ushort HeartbeatBaseId = 0x300;
    public const ushort ParamBaseId = 0x400;
    public const ushort TimeBaseId = 0x500;
    public const ushort ControlBaseId = 0x600;
}

public enum SscbSystemState : byte
{
    Normal = 0,
    FaultActive = 1,
    RecoverWait = 2,
    RecoverTry = 3,
    Lockout = 4
}

public enum SscbFaultCode : byte
{
    None = 0,
    Short = 1,
    Overload = 2,
    Overvoltage = 3,
    Overtemp = 4,
    Sample = 5,
    CommTimeout = 6,
    Selftest = 7
}

public enum SscbParamId : byte
{
    ShortThreshold = 0x01,
    OverCurrentThreshold = 0x02,
    OverVoltageThreshold = 0x03,
    OverTempThreshold = 0x04,
    I2TThreshold = 0x05,
    VoltageK = 0x06,
    CurrentK = 0x07,
    TempK = 0x08,
    NodeId = 0x09,
    RecoverMode = 0x0A
}

public enum SscbControlCommand : byte
{
    ClearFault = 0x01,
    SoftwareReset = 0x02,
    ReadFaultLog = 0x03,
    ClearFaultLog = 0x04
}
