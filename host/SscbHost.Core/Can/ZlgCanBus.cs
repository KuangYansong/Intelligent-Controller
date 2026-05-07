using System.Runtime.InteropServices;

namespace SscbHost.Core.Can;

public sealed class ZlgCanBus : ICanBus
{
    private const string DllName = "ControlCAN.dll";
    private const uint StatusOk = 1;
    private const uint ReceiveWaitMs = 50;

    private readonly object _sync = new();
    private CancellationTokenSource? _receiveCts;
    private Task? _receiveTask;
    private CanBusOptions? _options;

    public event EventHandler<CanFrame>? FrameReceived;
    public event EventHandler<string>? StatusChanged;

    public bool IsOpen { get; private set; }

    public Task OpenAsync(CanBusOptions options, CancellationToken cancellationToken = default)
    {
        lock (_sync)
        {
            if (IsOpen)
            {
                return Task.CompletedTask;
            }

            if (VCI_OpenDevice(options.DeviceType, options.DeviceIndex, 0) != StatusOk)
            {
                throw new InvalidOperationException("Failed to open ZLG CAN device. Check ControlCAN.dll, driver, device type, and USB connection.");
            }

            var init = VciInitConfig.For500K();
            if (VCI_InitCAN(options.DeviceType, options.DeviceIndex, options.ChannelIndex, ref init) != StatusOk)
            {
                VCI_CloseDevice(options.DeviceType, options.DeviceIndex);
                throw new InvalidOperationException("Failed to initialize ZLG CAN channel.");
            }

            if (VCI_StartCAN(options.DeviceType, options.DeviceIndex, options.ChannelIndex) != StatusOk)
            {
                VCI_CloseDevice(options.DeviceType, options.DeviceIndex);
                throw new InvalidOperationException("Failed to start ZLG CAN channel.");
            }

            _options = options;
            IsOpen = true;
            _receiveCts = new CancellationTokenSource();
            _receiveTask = Task.Run(() => ReceiveLoop(_receiveCts.Token), cancellationToken);
        }

        StatusChanged?.Invoke(this, "CAN channel opened.");
        return Task.CompletedTask;
    }

    public async Task CloseAsync()
    {
        CancellationTokenSource? cts;
        Task? task;
        CanBusOptions? options;

        lock (_sync)
        {
            if (!IsOpen)
            {
                return;
            }

            cts = _receiveCts;
            task = _receiveTask;
            options = _options;
            IsOpen = false;
            _receiveCts = null;
            _receiveTask = null;
            _options = null;
        }

        cts?.Cancel();
        if (task != null)
        {
            try
            {
                await task.ConfigureAwait(false);
            }
            catch (OperationCanceledException)
            {
            }
        }

        if (options != null)
        {
            VCI_CloseDevice(options.DeviceType, options.DeviceIndex);
        }

        cts?.Dispose();
        StatusChanged?.Invoke(this, "CAN channel closed.");
    }

    public Task SendAsync(CanFrame frame, CancellationToken cancellationToken = default)
    {
        if (!IsOpen || _options == null)
        {
            throw new InvalidOperationException("CAN channel is not open.");
        }

        var obj = VciCanObj.FromFrame(frame);
        var sent = VCI_Transmit(_options.DeviceType, _options.DeviceIndex, _options.ChannelIndex, new[] { obj }, 1);
        if (sent != 1)
        {
            throw new InvalidOperationException("Failed to transmit CAN frame.");
        }

        return Task.CompletedTask;
    }

    public void Dispose()
    {
        CloseAsync().GetAwaiter().GetResult();
    }

    private void ReceiveLoop(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            var options = _options;
            if (options == null)
            {
                return;
            }

            var count = VCI_GetReceiveNum(options.DeviceType, options.DeviceIndex, options.ChannelIndex);
            if (count == 0)
            {
                Thread.Sleep(10);
                continue;
            }

            var batch = Math.Min(count, 100);
            var objs = new VciCanObj[batch];
            var received = VCI_Receive(options.DeviceType, options.DeviceIndex, options.ChannelIndex, objs, batch, ReceiveWaitMs);
            for (var i = 0; i < received; i++)
            {
                FrameReceived?.Invoke(this, objs[i].ToFrame());
            }
        }
    }

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_OpenDevice(uint deviceType, uint deviceIndex, uint reserved);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_CloseDevice(uint deviceType, uint deviceIndex);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_InitCAN(uint deviceType, uint deviceIndex, uint canIndex, ref VciInitConfig config);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_StartCAN(uint deviceType, uint deviceIndex, uint canIndex);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_GetReceiveNum(uint deviceType, uint deviceIndex, uint canIndex);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_Transmit(uint deviceType, uint deviceIndex, uint canIndex, [In] VciCanObj[] send, uint len);

    [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
    private static extern uint VCI_Receive(uint deviceType, uint deviceIndex, uint canIndex, [Out] VciCanObj[] receive, uint len, uint waitTime);

    [StructLayout(LayoutKind.Sequential)]
    private struct VciInitConfig
    {
        public uint AccCode;
        public uint AccMask;
        public uint Reserved;
        public byte Filter;
        public byte Timing0;
        public byte Timing1;
        public byte Mode;

        public static VciInitConfig For500K() => new()
        {
            AccCode = 0,
            AccMask = 0xFFFFFFFF,
            Reserved = 0,
            Filter = 1,
            Timing0 = 0x00,
            Timing1 = 0x1C,
            Mode = 0
        };
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct VciCanObj
    {
        public uint ID;
        public uint TimeStamp;
        public byte TimeFlag;
        public byte SendType;
        public byte RemoteFlag;
        public byte ExternFlag;
        public byte DataLen;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
        public byte[] Data;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public byte[] Reserved;

        public static VciCanObj FromFrame(CanFrame frame) => new()
        {
            ID = frame.Id,
            SendType = 0,
            RemoteFlag = 0,
            ExternFlag = 0,
            DataLen = frame.Dlc,
            Data = frame.Data.ToArray(),
            Reserved = new byte[3]
        };

        public readonly CanFrame ToFrame()
        {
            var data = new byte[8];
            if (Data != null)
            {
                Array.Copy(Data, data, Math.Min(Data.Length, 8));
            }

            return new CanFrame((ushort)(ID & 0x7FF), DataLen, data);
        }
    }
}
