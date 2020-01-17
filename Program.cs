using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class serv
{
    public static void Main()
    {
        try
        {
            IPAddress ipAd = IPAddress.Parse("172.31.39.183");
            // use local m/c IP address, and 
            // use the same in the client

            /* Initializes the Listener */
            TcpListener myList = new TcpListener(ipAd, 8051);

            /* Start Listening at the specified port */
            myList.Start();

            Console.WriteLine("The server is running at port 8051...");
            Console.WriteLine("The local End point is  :" +
                              myList.LocalEndpoint);
            Console.WriteLine("Waiting for a connection.....");

            while (true)
            {
                Socket s = myList.AcceptSocket();
                Console.WriteLine("Connection accepted from " + s.RemoteEndPoint);

                byte[] b = new byte[100];
                int k = s.Receive(b);
                Console.WriteLine("Recieved...");
                for (int i = 0; i < k; i++)
                    Console.Write(Convert.ToChar(b[i]));

                ASCIIEncoding asen = new ASCIIEncoding();
                s.Send(asen.GetBytes("The string was recieved by the server."));
                Console.WriteLine("\nSent Acknowledgement");
                s.Close();
                myList.Stop();
                Thread.Sleep(1000);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine("Error..... " + e.StackTrace);
        }
    }
}