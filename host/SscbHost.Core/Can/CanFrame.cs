namespace SscbHost.Core.Can;

public readonly record struct CanFrame(ushort Id, byte Dlc, byte[] Data)
{
    public CanFrame(ushort id, ReadOnlySpan<byte> data)
        : this(id, (byte)Math.Min(data.Length, 8), Pad(data))
    {
    }

    private static byte[] Pad(ReadOnlySpan<byte> data)
    {
        var padded = new byte[8];
        data[..Math.Min(data.Length, 8)].CopyTo(padded);
        return padded;
    }
}
