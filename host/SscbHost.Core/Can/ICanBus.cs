namespace SscbHost.Core.Can;

public interface ICanBus : IDisposable
{
    event EventHandler<CanFrame>? FrameReceived;
    event EventHandler<string>? StatusChanged;

    bool IsOpen { get; }

    Task OpenAsync(CanBusOptions options, CancellationToken cancellationToken = default);
    Task CloseAsync();
    Task SendAsync(CanFrame frame, CancellationToken cancellationToken = default);
}

public sealed record CanBusOptions(
    uint DeviceType,
    uint DeviceIndex,
    uint ChannelIndex,
    uint Bitrate);
