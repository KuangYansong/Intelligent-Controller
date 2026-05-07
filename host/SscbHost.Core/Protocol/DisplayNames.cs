namespace SscbHost.Core.Protocol;

public static class DisplayNames
{
    public static string FaultName(SscbFaultCode code) => code switch
    {
        SscbFaultCode.None => "无故障",
        SscbFaultCode.Short => "短路故障",
        SscbFaultCode.Overload => "过载故障",
        SscbFaultCode.Overvoltage => "过压故障",
        SscbFaultCode.Overtemp => "过温故障",
        SscbFaultCode.Sample => "采样异常",
        SscbFaultCode.CommTimeout => "通信超时",
        SscbFaultCode.Selftest => "自检失败",
        _ => $"未知故障 0x{(byte)code:X2}"
    };

    public static string ParamName(SscbParamId id) => id switch
    {
        SscbParamId.ShortThreshold => "短路阈值",
        SscbParamId.OverCurrentThreshold => "过流阈值",
        SscbParamId.OverVoltageThreshold => "过压阈值",
        SscbParamId.OverTempThreshold => "过温阈值",
        SscbParamId.I2TThreshold => "I2T 阈值",
        SscbParamId.VoltageK => "电压系数",
        SscbParamId.CurrentK => "电流系数",
        SscbParamId.TempK => "温度系数",
        SscbParamId.NodeId => "节点 ID",
        SscbParamId.RecoverMode => "恢复模式",
        _ => $"参数 0x{(byte)id:X2}"
    };
}
