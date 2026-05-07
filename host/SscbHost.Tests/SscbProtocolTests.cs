using SscbHost.Core.Can;
using SscbHost.Core.Protocol;
using Xunit;

namespace SscbHost.Tests;

public sealed class SscbProtocolTests
{
    [Fact]
    public void ParsesRealtimeFrame()
    {
        var frame = new CanFrame(0x101, new byte[] { 0x40, 0x1F, 0x73, 0x00, 0x2C, 0x01, 0x01, 0x00 });

        Assert.True(SscbProtocol.TryParseRealtime(frame, 1, out var data));
        Assert.Equal(800.0, data.VoltageV, 1);
        Assert.Equal(11.5, data.CurrentA, 1);
        Assert.Equal(30.0, data.TemperatureC, 1);
        Assert.True(data.Status.HasFlag(SscbStatusBits.Normal));
    }

    [Fact]
    public void ParsesFaultFrame()
    {
        var frame = new CanFrame(0x201, new byte[] { 0x02, 0x78, 0x56, 0x34, 0x12, 0xC8, 0x00, 0x50 });

        Assert.True(SscbProtocol.TryParseFault(frame, 1, out var data));
        Assert.Equal(SscbFaultCode.Overload, data.Fault);
        Assert.Equal(0x12345678u, data.TimestampMs);
        Assert.Equal(20.0, data.CurrentA, 1);
        Assert.Equal(800.0, data.VoltageLite, 1);
    }

    [Fact]
    public void ParsesHeartbeatFrame()
    {
        var frame = new CanFrame(0x301, new byte[] { 0x01, 0x04, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x00 });

        Assert.True(SscbProtocol.TryParseHeartbeat(frame, 1, out var data));
        Assert.Equal(1, data.NodeId);
        Assert.Equal(SscbSystemState.Lockout, data.State);
        Assert.Equal(42ul, data.RuntimeSeconds);
    }

    [Fact]
    public void BuildsParamWriteLittleEndianFloat()
    {
        var frame = SscbProtocol.BuildParamWrite(1, SscbParamId.OverCurrentThreshold, 80.0f);

        Assert.Equal(0x401, frame.Id);
        Assert.Equal(0x11, frame.Data[0]);
        Assert.Equal((byte)SscbParamId.OverCurrentThreshold, frame.Data[1]);
        Assert.Equal(80.0f, BitConverter.ToSingle(frame.Data, 2));
    }

    [Fact]
    public void ParsesParamResponse()
    {
        var data = new byte[8];
        data[0] = 0;
        data[1] = (byte)SscbParamId.ShortThreshold;
        BitConverter.GetBytes(200.0f).CopyTo(data, 2);

        Assert.True(SscbProtocol.TryParseParamResponse(new CanFrame(0x401, data), 1, out var response));
        Assert.True(response.Ok);
        Assert.Equal(SscbParamId.ShortThreshold, response.ParamId);
        Assert.Equal(200.0f, response.Value);
    }

    [Fact]
    public void BuildsControlFrame()
    {
        var frame = SscbProtocol.BuildControl(1, SscbControlCommand.ReadFaultLog);

        Assert.Equal(0x601, frame.Id);
        Assert.Equal(0x03, frame.Data[0]);
    }
}
