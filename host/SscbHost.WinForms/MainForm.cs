using System.ComponentModel;
using SscbHost.Core.Can;
using SscbHost.Core.Protocol;

namespace SscbHost.WinForms;

public sealed class MainForm : Form
{
    private readonly BindingList<ParamRow> _params = new();
    private readonly BindingList<FaultRow> _faults = new();
    private readonly ICanBus _canBus = new ZlgCanBus();

    private NumericUpDown _nodeId = null!;
    private NumericUpDown _deviceType = null!;
    private NumericUpDown _deviceIndex = null!;
    private NumericUpDown _channelIndex = null!;
    private Button _connectButton = null!;
    private Button _disconnectButton = null!;
    private Label _connectionStatus = null!;
    private Label _voltage = null!;
    private Label _current = null!;
    private Label _temperature = null!;
    private Label _state = null!;
    private Label _runtime = null!;
    private CheckedListBox _statusBits = null!;
    private DataGridView _paramGrid = null!;
    private DataGridView _faultGrid = null!;
    private TextBox _log = null!;

    public MainForm()
    {
        Text = "智能固态断路器 CAN 上位机";
        MinimumSize = new Size(1180, 760);
        StartPosition = FormStartPosition.CenterScreen;

        BuildParameterRows();
        BuildLayout();

        _canBus.FrameReceived += OnFrameReceived;
        _canBus.StatusChanged += (_, message) => BeginInvoke(() => AppendLog(message));
        FormClosing += async (_, _) => await _canBus.CloseAsync();
    }

    private byte NodeId => (byte)_nodeId.Value;

    private void BuildLayout()
    {
        var root = new TableLayoutPanel
        {
            Dock = DockStyle.Fill,
            ColumnCount = 2,
            RowCount = 4,
            Padding = new Padding(12),
        };
        root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 42));
        root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 58));
        root.RowStyles.Add(new RowStyle(SizeType.Absolute, 92));
        root.RowStyles.Add(new RowStyle(SizeType.Absolute, 170));
        root.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        root.RowStyles.Add(new RowStyle(SizeType.Absolute, 110));
        Controls.Add(root);

        root.Controls.Add(BuildConnectionPanel(), 0, 0);
        root.SetColumnSpan(root.Controls[^1], 2);
        root.Controls.Add(BuildRealtimePanel(), 0, 1);
        root.Controls.Add(BuildCommandPanel(), 1, 1);
        root.Controls.Add(BuildParamPanel(), 0, 2);
        root.Controls.Add(BuildFaultPanel(), 1, 2);
        root.Controls.Add(BuildLogPanel(), 0, 3);
        root.SetColumnSpan(root.Controls[^1], 2);
    }

    private Control BuildConnectionPanel()
    {
        var panel = new GroupBox { Text = "连接", Dock = DockStyle.Fill };
        var flow = new FlowLayoutPanel { Dock = DockStyle.Fill, Padding = new Padding(8), WrapContents = false };
        panel.Controls.Add(flow);

        _deviceType = AddNumber(flow, "设备类型", 4, 0, 100);
        _deviceIndex = AddNumber(flow, "设备索引", 0, 0, 16);
        _channelIndex = AddNumber(flow, "通道", 0, 0, 8);
        _nodeId = AddNumber(flow, "NodeID", 1, 1, 127);
        _connectButton = AddButton(flow, "连接", ConnectAsync);
        _disconnectButton = AddButton(flow, "断开", DisconnectAsync);
        _connectionStatus = new Label { Text = "未连接", AutoSize = true, Padding = new Padding(12, 9, 0, 0) };
        flow.Controls.Add(_connectionStatus);
        return panel;
    }

    private Control BuildRealtimePanel()
    {
        var panel = new GroupBox { Text = "实时数据", Dock = DockStyle.Fill };
        var grid = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 4, Padding = new Padding(10) };
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90));
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        panel.Controls.Add(grid);

        _voltage = AddMetric(grid, 0, "电压");
        _current = AddMetric(grid, 1, "电流");
        _temperature = AddMetric(grid, 2, "温度");
        _runtime = AddMetric(grid, 3, "运行时间");
        return panel;
    }

    private Control BuildCommandPanel()
    {
        var panel = new GroupBox { Text = "状态与命令", Dock = DockStyle.Fill };
        var grid = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2, RowCount = 2, Padding = new Padding(10) };
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 45));
        grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 55));
        panel.Controls.Add(grid);

        _statusBits = new CheckedListBox { Dock = DockStyle.Fill, Enabled = false };
        _statusBits.Items.AddRange(new object[] { "系统正常", "短路故障", "过载故障", "过压故障", "过温故障", "闭锁状态", "自检失败" });
        grid.Controls.Add(_statusBits, 0, 0);
        grid.SetRowSpan(_statusBits, 2);

        var commandFlow = new FlowLayoutPanel { Dock = DockStyle.Fill };
        grid.Controls.Add(commandFlow, 1, 0);
        AddButton(commandFlow, "同步时间", () => SendFrameAsync(SscbProtocol.BuildTimeSync(NodeId, DateTimeOffset.Now)));
        AddButton(commandFlow, "清故障", () => SendControlAsync(SscbControlCommand.ClearFault));
        AddButton(commandFlow, "软件复位", () => SendControlAsync(SscbControlCommand.SoftwareReset));
        AddButton(commandFlow, "读取故障记录", () => SendControlAsync(SscbControlCommand.ReadFaultLog));
        AddButton(commandFlow, "清空故障记录", () => SendControlAsync(SscbControlCommand.ClearFaultLog));

        _state = new Label { Text = "状态：未知", AutoSize = true, Dock = DockStyle.Fill };
        grid.Controls.Add(_state, 1, 1);
        return panel;
    }

    private Control BuildParamPanel()
    {
        var panel = new GroupBox { Text = "参数", Dock = DockStyle.Fill };
        var layout = new TableLayoutPanel { Dock = DockStyle.Fill, RowCount = 2 };
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        layout.RowStyles.Add(new RowStyle(SizeType.Absolute, 44));
        panel.Controls.Add(layout);

        _paramGrid = new DataGridView
        {
            Dock = DockStyle.Fill,
            AutoGenerateColumns = false,
            DataSource = _params,
            AllowUserToAddRows = false,
            SelectionMode = DataGridViewSelectionMode.FullRowSelect
        };
        _paramGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(ParamRow.Id), HeaderText = "ID", Width = 52, ReadOnly = true });
        _paramGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(ParamRow.Name), HeaderText = "名称", Width = 120, ReadOnly = true });
        _paramGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(ParamRow.Value), HeaderText = "值", AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill });
        _paramGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(ParamRow.Status), HeaderText = "状态", Width = 80, ReadOnly = true });
        layout.Controls.Add(_paramGrid, 0, 0);

        var buttons = new FlowLayoutPanel { Dock = DockStyle.Fill };
        layout.Controls.Add(buttons, 0, 1);
        AddButton(buttons, "读取选中", ReadSelectedParamAsync);
        AddButton(buttons, "写入选中", WriteSelectedParamAsync);
        AddButton(buttons, "读取全部", ReadAllParamsAsync);
        return panel;
    }

    private Control BuildFaultPanel()
    {
        var panel = new GroupBox { Text = "故障记录", Dock = DockStyle.Fill };
        _faultGrid = new DataGridView
        {
            Dock = DockStyle.Fill,
            AutoGenerateColumns = false,
            DataSource = _faults,
            AllowUserToAddRows = false,
            ReadOnly = true
        };
        _faultGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(FaultRow.Time), HeaderText = "接收时间", Width = 150 });
        _faultGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(FaultRow.Fault), HeaderText = "故障", Width = 110 });
        _faultGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(FaultRow.Timestamp), HeaderText = "时间戳(ms)", Width = 110 });
        _faultGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(FaultRow.Current), HeaderText = "电流(A)", Width = 90 });
        _faultGrid.Columns.Add(new DataGridViewTextBoxColumn { DataPropertyName = nameof(FaultRow.Voltage), HeaderText = "电压(V)", Width = 90 });
        panel.Controls.Add(_faultGrid);
        return panel;
    }

    private Control BuildLogPanel()
    {
        var panel = new GroupBox { Text = "通信日志", Dock = DockStyle.Fill };
        _log = new TextBox { Dock = DockStyle.Fill, Multiline = true, ScrollBars = ScrollBars.Vertical, ReadOnly = true };
        panel.Controls.Add(_log);
        return panel;
    }

    private void BuildParameterRows()
    {
        foreach (SscbParamId id in Enum.GetValues<SscbParamId>())
        {
            _params.Add(new ParamRow(id, DisplayNames.ParamName(id), 0, "未读取"));
        }
    }

    private static NumericUpDown AddNumber(FlowLayoutPanel flow, string label, decimal value, decimal min, decimal max)
    {
        flow.Controls.Add(new Label { Text = label, AutoSize = true, Padding = new Padding(8, 9, 0, 0) });
        var input = new NumericUpDown { Minimum = min, Maximum = max, Value = value, Width = 72 };
        flow.Controls.Add(input);
        return input;
    }

    private static Button AddButton(FlowLayoutPanel flow, string text, Func<Task> action)
    {
        var button = new Button { Text = text, AutoSize = true, Height = 32 };
        button.Click += async (_, _) =>
        {
            button.Enabled = false;
            try
            {
                await action();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "操作失败", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally
            {
                button.Enabled = true;
            }
        };
        flow.Controls.Add(button);
        return button;
    }

    private static Label AddMetric(TableLayoutPanel grid, int row, string name)
    {
        grid.Controls.Add(new Label { Text = name, Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft }, 0, row);
        var value = new Label { Text = "--", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft, Font = new Font("Microsoft YaHei UI", 12, FontStyle.Bold) };
        grid.Controls.Add(value, 1, row);
        return value;
    }

    private async Task ConnectAsync()
    {
        await _canBus.OpenAsync(new CanBusOptions((uint)_deviceType.Value, (uint)_deviceIndex.Value, (uint)_channelIndex.Value, 500000));
        _connectionStatus.Text = "已连接";
    }

    private async Task DisconnectAsync()
    {
        await _canBus.CloseAsync();
        _connectionStatus.Text = "未连接";
    }

    private Task SendControlAsync(SscbControlCommand command) => SendFrameAsync(SscbProtocol.BuildControl(NodeId, command));

    private async Task SendFrameAsync(CanFrame frame)
    {
        await _canBus.SendAsync(frame);
        AppendLog($"TX 0x{frame.Id:X3} {BitConverter.ToString(frame.Data, 0, frame.Dlc)}");
    }

    private Task ReadSelectedParamAsync()
    {
        if (_paramGrid.CurrentRow?.DataBoundItem is ParamRow row)
        {
            return SendFrameAsync(SscbProtocol.BuildParamRead(NodeId, row.Id));
        }

        return Task.CompletedTask;
    }

    private Task WriteSelectedParamAsync()
    {
        if (_paramGrid.CurrentRow?.DataBoundItem is ParamRow row)
        {
            return SendFrameAsync(SscbProtocol.BuildParamWrite(NodeId, row.Id, row.Value));
        }

        return Task.CompletedTask;
    }

    private async Task ReadAllParamsAsync()
    {
        foreach (var row in _params)
        {
            await SendFrameAsync(SscbProtocol.BuildParamRead(NodeId, row.Id));
            await Task.Delay(20);
        }
    }

    private void OnFrameReceived(object? sender, CanFrame frame)
    {
        BeginInvoke(() =>
        {
            AppendLog($"RX 0x{frame.Id:X3} {BitConverter.ToString(frame.Data, 0, frame.Dlc)}");
            if (SscbProtocol.TryParseRealtime(frame, NodeId, out var realtime))
            {
                _voltage.Text = $"{realtime.VoltageV:F1} V";
                _current.Text = $"{realtime.CurrentA:F1} A";
                _temperature.Text = $"{realtime.TemperatureC:F1} ℃";
                UpdateStatus(realtime.Status);
                return;
            }

            if (SscbProtocol.TryParseHeartbeat(frame, NodeId, out var heartbeat))
            {
                _state.Text = $"状态：{heartbeat.State}";
                _runtime.Text = $"{heartbeat.RuntimeSeconds} s";
                return;
            }

            if (SscbProtocol.TryParseFault(frame, NodeId, out var fault))
            {
                _faults.Insert(0, new FaultRow(DateTime.Now.ToString("HH:mm:ss.fff"), DisplayNames.FaultName(fault.Fault), fault.TimestampMs, fault.CurrentA, fault.VoltageLite));
                return;
            }

            if (SscbProtocol.TryParseParamResponse(frame, NodeId, out var param))
            {
                var row = _params.FirstOrDefault(p => p.Id == param.ParamId);
                if (row != null)
                {
                    row.Value = param.Value;
                    row.Status = param.Ok ? "OK" : "失败";
                    _paramGrid.Refresh();
                }
            }
        });
    }

    private void UpdateStatus(SscbStatusBits bits)
    {
        var values = new[]
        {
            SscbStatusBits.Normal,
            SscbStatusBits.ShortFault,
            SscbStatusBits.OverloadFault,
            SscbStatusBits.OvervoltageFault,
            SscbStatusBits.OvertempFault,
            SscbStatusBits.Lockout,
            SscbStatusBits.SelftestFailed
        };

        for (var i = 0; i < values.Length; i++)
        {
            _statusBits.SetItemChecked(i, bits.HasFlag(values[i]));
        }
    }

    private void AppendLog(string message)
    {
        _log.AppendText($"{DateTime.Now:HH:mm:ss.fff} {message}{Environment.NewLine}");
    }

    private sealed class ParamRow(SscbParamId id, string name, float value, string status)
    {
        public SscbParamId Id { get; } = id;
        public string Name { get; } = name;
        public float Value { get; set; } = value;
        public string Status { get; set; } = status;
    }

    private sealed class FaultRow(string time, string fault, uint timestamp, double current, double voltage)
    {
        public string Time { get; } = time;
        public string Fault { get; } = fault;
        public uint Timestamp { get; } = timestamp;
        public double Current { get; } = current;
        public double Voltage { get; } = voltage;
    }
}
