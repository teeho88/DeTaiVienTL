using Microsoft.Win32;
using OfficeOpenXml;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;

namespace ControlApp.ViewModels
{
    public class MainViewModel : ViewModelBase
    {
        public MainViewModel()
        {
            LoadedWindowCommand = new RelayCommand(x =>
            {
                Rate = "115200";
                OnPropertyChanged("Rate");
                ReadContent = "READ";
                OnPropertyChanged("ReadContent");
                ConnectBT_Text = "CONNECT";
                CalibRFC_Text = "CALIB RFC";
            });
            ConnectCM = new RelayCommand(x =>
            {
                if (ComName == null)
                {
                    MessageBox.Show("COM = ???");
                    return;
                }

                if (!serial.IsOpen)
                {
                    serial.PortName = ComName;
                    serial.BaudRate = int.Parse(Rate);
                    try
                    {
                        serial.Open();
                    }
                    catch(UnauthorizedAccessException)
                    {
                        MessageBox.Show(ComName + " is busy");
                        return;
                    }
                    ConnectBT_Text = "CLOSE";
                    MessageBox.Show("Connected");
                }
                else
                {
                    serial.Close();
                    ConnectBT_Text = "CONNECT";
                    CalibRFC_Text = "CALIB RFC";
                    ReadContent = "READ";
                    serial.DataReceived -= SerialHandlerRecord;
                    Text2Show = "";
                    MessageBox.Show("Closed");
                }
            });
            CalibIMU_BT = new RelayCommand(x =>
            {
                if(!serial.IsOpen)
                {
                    MessageBox.Show("Please connect COM port");
                    return;
                }
                Text2Show = "Hieu chinh IMU50......\r\n";
                serial.DataReceived -= SerialHandlerRecord;
                serial.DataReceived += IMU_Calib;
                SerialHandlerRecord = IMU_Calib;
                serial.Write("IMUCS");
            });
            CalibRFC_BT = new RelayCommand(x =>
            {
                if (!serial.IsOpen)
                {
                    MessageBox.Show("Please connect COM port");
                    return;
                }
                if (CalibRFC_Text == "CALIB RFC")
                {
                    RFCValue = new List<int>();
                    Text2Show = "Hieu chinh RFC......\r\n(Xoay deu RFC vai vong. An STOP CALIB RFC khi muon ket thuc)\r\n";
                    serial.DataReceived -= SerialHandlerRecord;
                    serial.DataReceived += RFC_Calib;
                    SerialHandlerRecord = RFC_Calib;
                    serial.Write("RFCCS");
                    CalibRFC_Text = "STOP CALIB RFC";
                }
                else
                {
                    int minRFC = RFCValue.Min();
                    int maxRFC = RFCValue.Max();
                    serial.Write("RFCCE" + minRFC.ToString("0000") + maxRFC.ToString("0000"));
                    CalibRFC_Text = "CALIB RFC";
                    Text2Show = String.Format(">Ket thuc hieu chinh RFC:\r\n * Min RFC = {0}\r\n * Max RFC = {1}",minRFC,maxRFC);
                }

            });
            ComNameDropdown = new RelayCommand(x =>
            {
                COMList = new List<string>(SerialPort.GetPortNames());
                OnPropertyChanged("COMList");
            });
            ReadCM = new RelayCommand(x =>
            {
                if (ReadContent == "READ")
                {
                    if (!serial.IsOpen)
                    {
                        MessageBox.Show("Please connect COM port");
                        return;
                    }
                    dataReceivStr = "";
                    ReadContent = "STOP";
                    OnPropertyChanged("ReadContent");
                    serial.DataReceived -= SerialHandlerRecord;
                    serial.DataReceived += Serial_DataReceived;
                    SerialHandlerRecord = Serial_DataReceived;
                    Text2Show = "Reading data...\r\nPush Stop button when you want to finish collecting data";
                }
                else
                {
                    ReadContent = "READ";
                    OnPropertyChanged("ReadContent");
                    serial.DataReceived -= Serial_DataReceived;
                    Text2Show = "Reading completed\r\nPush Save button to save data";
                }
            });
            SaveCM = new RelayCommand(x =>
            {
                SaveFile();
            });
            ClearTextCM = new RelayCommand(x =>
            {
                Text2Show = "";
            });
        }

        private void IMU_Calib(object sender, SerialDataReceivedEventArgs e)
        {
            Text2Show += serial.ReadExisting();
        }

        private void RFC_Calib(object sender, SerialDataReceivedEventArgs e)
        {
            string comingStr = serial.ReadExisting();
            dataReceivStr = comingStr;
            int startIdx = dataReceivStr.IndexOf('>');
            dataReceivStr = dataReceivStr.Substring(startIdx + 1);
            int endIdx = dataReceivStr.IndexOf('\r');
            string tempStr = dataReceivStr.Substring(0, endIdx);
            int tempInt;
            if (int.TryParse(tempStr, out tempInt))
            {
                RFCValue.Add(tempInt);
                Text2Show = tempStr;
            }
            else
            {
                Text2Show = comingStr;
            }
        }

        private void Serial_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                string str = serial.ReadTo("\r");
                dataReceivStr += str;
                int idx = str.IndexOf('>');
                AxisRoll = str.Substring(idx + 1, 5);
            }
            catch { }
        }

        private void SaveFile()
        {
            if(dataReceivStr.Length == 0)
            {
                MessageBox.Show("Chua co data");
                return;
            }
            string filePath = "";
            // tạo SaveFileDialog để lưu file excel
            SaveFileDialog dialog = new SaveFileDialog();

            // chỉ lọc ra các file có định dạng Excel
            dialog.Filter = "Excel | *.xlsx | Excel 2003 | *.xls";

            // Nếu mở file và chọn nơi lưu file thành công sẽ lưu đường dẫn lại dùng
            if (dialog.ShowDialog() == true)
            {
                filePath = dialog.FileName;
            }

            // nếu đường dẫn null hoặc rỗng thì báo không hợp lệ và return hàm
            if (string.IsNullOrEmpty(filePath))
            {
                return;
            }

            try
            {
                ExcelPackage.LicenseContext = LicenseContext.NonCommercial;
                using (ExcelPackage p = new ExcelPackage())
                {
                    // đặt tên người tạo file
                    p.Workbook.Properties.Author = "Teeho";

                    //Tạo một sheet để làm việc trên đó
                    p.Workbook.Worksheets.Add("Data");

                    // lấy sheet vừa add ra để thao tác
                    ExcelWorksheet ws = p.Workbook.Worksheets[0];

                    // đặt tên cho sheet
                    ws.Name = "Data";
                    // fontsize mặc định cho cả sheet
                    ws.Cells.Style.Font.Size = 11;
                    // font family mặc định cho cả sheet
                    ws.Cells.Style.Font.Name = "Calibri";

                    // Tạo danh sách các column header
                    string[] arrColumnHeader = {"Roll",
                                                "Gyr_z",
                                                "ax",
                                                "ay",
                                                "interval"
                    };

                    // lấy ra số lượng cột cần dùng dựa vào số lượng header
                    var countColHeader = arrColumnHeader.Length;

                    //tạo các header từ column header đã tạo từ bên trên
                    int rowIndex = 1;
                    int colIndex = 1;
                    foreach (var item in arrColumnHeader)
                    {
                        var cell = ws.Cells[rowIndex, colIndex];

                        //gán giá trị
                        cell.Value = item;

                        colIndex++;
                    }

                    List<string> dataListStr = new List<string>(
                        dataReceivStr.Split('\r')
                        );


                    // với mỗi item trong danh sách sẽ ghi trên 1 dòng
                    foreach (var item in dataListStr)
                    {
                        if (item.Length == 0)
                            continue;
                        if (item[0] != '>')
                            continue;

                        //gán giá trị cho từng cell
                        string[] data = item.Remove(0, 1).Split('|');
                        if (data.Length != arrColumnHeader.Length)
                            continue;
                        // rowIndex tương ứng từng dòng dữ liệu
                        rowIndex++;
                        for (colIndex = 1; colIndex < arrColumnHeader.Length+1; colIndex++)
                        {
                            ws.Cells[rowIndex, colIndex].Value = double.Parse(data[colIndex - 1]);
                        }
                    }

                    //Lưu file lại
                    Byte[] bin = p.GetAsByteArray();
                    File.WriteAllBytes(filePath, bin);
                }
                MessageBox.Show("Xuất excel thành công!");
            }
            catch (Exception e)
            {
                MessageBox.Show("Có lỗi khi lưu file!");
            }
        }

        #region variables
        //Serial 
        static SerialPort serial = new SerialPort();
        private string dataReceivStr = "";
        private string text2Show;
        private string connectBT_Text;
        private string calibRFC_Text;
        private string readContent;
        public string ComName { get; set; }
        public string Rate { get; set; }
        private string axisRoll;
        public RelayCommand ConnectCM { get; set; }
        public RelayCommand CalibIMU_BT { get; set; }
        public RelayCommand CalibRFC_BT { get; set; }
        public RelayCommand LoadedWindowCommand { get; set; }
        public RelayCommand ComNameDropdown { get; set; }
        public RelayCommand ReadCM { get; set; }
        public RelayCommand SaveCM { get; set; }
        public RelayCommand ClearTextCM { get; set; }
        public List<string> COMList { get; set; }
        public string Text2Show { get => text2Show; set => SetProperty(ref text2Show, value); }
        public string AxisRoll { get => axisRoll; set => SetProperty(ref axisRoll, value); }
        public string ConnectBT_Text { get => connectBT_Text; set => SetProperty(ref connectBT_Text, value); }
        public string CalibRFC_Text { get => calibRFC_Text; set => SetProperty(ref calibRFC_Text, value); }
        public string ReadContent { get => readContent; set => SetProperty(ref readContent, value); }

        private List<int> RFCValue;
        private SerialDataReceivedEventHandler SerialHandlerRecord;
        #endregion

    }
}
