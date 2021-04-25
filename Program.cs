using System;
using System.IO;
using System.IO.Pipes;
using System.Text;


namespace ConsoleApplication1
{

    class PipeClient
    {
        static void Main()
        {

            Console.WriteLine("Введите имя канала: ");
            string pipeName = Console.ReadLine();
            using (var pipe = new NamedPipeClientStream("localhost", pipeName, PipeDirection.InOut))
            {
                pipe.Connect(5000);
                pipe.ReadMode = PipeTransmissionMode.Message;      //режим передачи канала
                do
                {
                    Console.Write("client> ");
                    var input = Console.ReadLine();         //ввод от клиента
                    if (string.IsNullOrEmpty(input))
                        continue;
                    byte[] bytes = Encoding.Default.GetBytes(input);
                    pipe.Write(bytes, 0, bytes.Length);
                    if (input.ToLower() == "exit")
                        return;

                    var result = ReadMessage(pipe);
                    Console.WriteLine(Encoding.UTF8.GetString(result));     
                    Console.WriteLine();


                } while (true);
            }


        }
        private static byte[] ReadMessage(PipeStream pipe)
        {
            byte[] buffer = new byte[1024];
            using (var ms = new MemoryStream())
            {
                do
                {
                    var readBytes = pipe.Read(buffer, 0, buffer.Length);
                    ms.Write(buffer, 0, readBytes);
                }
                while (!pipe.IsMessageComplete);

                return ms.ToArray();
            }
        }
    }
    


}

