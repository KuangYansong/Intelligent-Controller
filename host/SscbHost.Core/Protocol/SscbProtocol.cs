using System.Buffers.Binary;
using SscbHost.Core.Can;

namespace SscbHost.Core.Protocol;

public static class SscbProtocol
{
    public static ushort MakeId(ushort baseId, byte nodeId) => (ushort)(baseId + nodeId);

    public static bool TryParseRealtime(CanFrame frame, byte nodeId, out RealtimeData data)
    {
        data = default!;
        if (frame.Id != MakeId(SscbConstants.RealtimeBaseId, nodeId) || frame.Dlc < 8)
        {
            return false;
        }

        data = new RealtimeData(
            ReadUInt16(frame.Data, 0) / 10.0,
            ReadUInt16(frame.Data, 2) / 10.0,
            unchecked((short)ReadUInt16(frame.Data, 4)) / 10.0,
            (SscbStatusBits)frame.Data[6]);
        return true;
    }

    public static bool TryParseFault(CanFrame frame, byte nodeId, out FaultData data)
    {
        data = default!;
        if (frame.Id != MakeId(SscbConstants.FaultBaseId, nodeId) || frame.Dlc < 8)
        {
            return false;
        }

        data = new FaultData(
            (SscbFaultCode)frame.Data[0],
            ReadUInt32(frame.Data, 1),
            ReadUInt16(frame.Data, 5) / 10.0,
            frame.Data[7] * 10.0);
        return true;
    }

    public static bool TryParseHeartbeat(CanFrame frame, byte nodeId, out HeartbeatData data)
    {
        data = default!;
        if (frame.Id != MakeId(SscbConstants.HeartbeatBaseId, nodeId) || frame.Dlc < 8)
        {
            return false;
        }

        ulong runtime = 0;
        for (var i = 0; i < 6; i++)
        {
            runtime |= (ulong)frame.Data[2 + i] << (8 * i);
        }

        data = new HeartbeatData(frame.Data[0], (SscbSystemState)frame.Data[1], runtime);
        return true;
    }

    public static bool TryParseParamResponse(CanFrame frame, byte nodeId, out ParamResponse response)
    {
        response = default!;
        if (frame.Id != MakeId(SscbConstants.ParamBaseId, nodeId) || frame.Dlc < 8)
        {
            return false;
        }

        response = new ParamResponse(
            frame.Data[0] == 0,
            (SscbParamId)frame.Data[1],
            BitConverter.Int32BitsToSingle((int)ReadUInt32(frame.Data, 2)));
        return true;
    }

    public static CanFrame BuildTimeSync(byte nodeId, DateTimeOffset now)
    {
        var data = new byte[8];
        var unixSec = (uint)now.ToUnixTimeSeconds();
        var millis = (uint)now.Millisecond;
        WriteUInt32(data, 0, unixSec);
        WriteUInt32(data, 4, millis);
        return new CanFrame(MakeId(SscbConstants.TimeBaseId, nodeId), data);
    }

    public static CanFrame BuildControl(byte nodeId, SscbControlCommand command)
    {
        var data = new byte[8];
        data[0] = (byte)command;
        return new CanFrame(MakeId(SscbConstants.ControlBaseId, nodeId), data);
    }

    public static CanFrame BuildParamRead(byte nodeId, SscbParamId paramId)
    {
        var data = new byte[8];
        data[0] = 0x10;
        data[1] = (byte)paramId;
        return new CanFrame(MakeId(SscbConstants.ParamBaseId, nodeId), data);
    }

    public static CanFrame BuildParamWrite(byte nodeId, SscbParamId paramId, float value)
    {
        var data = new byte[8];
        data[0] = 0x11;
        data[1] = (byte)paramId;
        WriteUInt32(data, 2, (uint)BitConverter.SingleToInt32Bits(value));
        return new CanFrame(MakeId(SscbConstants.ParamBaseId, nodeId), data);
    }

    private static ushort ReadUInt16(byte[] data, int offset) => BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(offset, 2));

    private static uint ReadUInt32(byte[] data, int offset) => BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(offset, 4));

    private static void WriteUInt32(byte[] data, int offset, uint value) => BinaryPrimitives.WriteUInt32LittleEndian(data.AsSpan(offset, 4), value);
}
