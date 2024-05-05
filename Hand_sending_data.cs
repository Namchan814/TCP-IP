using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Leap;
using Leap.Unity;
using System.Threading;
using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using UnityEngine.UI;
using JetBrains.Annotations;
using Unity.VisualScripting;
using UnityEditor.SceneManagement;

public class Hand_sending_data : PostProcessProvider
{
    Thread thread;
    [SerializeField]
    private string connectionIP = "127.0.0.1";
    [SerializeField]
    private int connectionPort = 1111;
    IPAddress address;
    TcpClient tcpClient;
    TcpListener listener;

    public Text HandPos;
    public Text TouchMessage;
    bool running;

    private float handPositionX = 0f;
    private float handPositionY = 0f;
    private float handPositionZ = 0f;

    private void Start()
    {
        ThreadStart start = new ThreadStart(GetInfo);
        thread = new Thread(start);
        thread.Start();
    }

    void GetInfo()
    {
        address = IPAddress.Parse(connectionIP);
        listener = new TcpListener(IPAddress.Any, connectionPort);
        listener.Start();
        tcpClient = listener.AcceptTcpClient();
        running = true;
        while (running)
        {
            SendData();
        }
    }
    private void SendData()
    {
        NetworkStream stream = tcpClient.GetStream();
        /*
        print((handPositionX, handPositionY, handPositionZ));

        byte[] myWriteBuffer = Encoding.UTF8.GetBytes("Hand Position: ");
        stream.Write(myWriteBuffer, 0, myWriteBuffer.Length);

        byte[] handPosition = Encoding.UTF8.GetBytes((handPositionX, handPositionY, handPositionZ).ToString());
        stream.Write(handPosition, 0, handPosition.Length);
        */
        if(Collide.IsTouched == true)
        {
            byte[] TouchCondition = Encoding.UTF8.GetBytes("1");
            stream.Write(TouchCondition, 0, TouchCondition.Length);
        }
        else if(Collide.IsTouched == false)
        {
            byte[] TouchCondition = Encoding.UTF8.GetBytes("0");
            stream.Write(TouchCondition, 0, TouchCondition.Length);
        }
    }
    void FixedUpdate()
    {
        //print((handPositionX, handPositionY, handPositionZ));

    }

    public override void ProcessFrame(ref Frame inputFrame)
    {
        foreach (var hand in inputFrame.Hands)
        {
            Vector3 HandPosition = hand.PalmPosition;

            handPositionX = HandPosition.x;
            handPositionY = HandPosition.y;
            handPositionZ = HandPosition.z;

            //print((handPositionX, handPositionY, handPositionY));
        }
    }
    public void OnGUI() 
    {
        HandPos.text = "Your hand position is: " + (handPositionX, handPositionY, handPositionZ).ToString();    
        
        if (Collide.IsTouched == true)
        {
            TouchMessage.text = "You have touched the object";
        }
        else if (Collide.IsTouched == false)
        {
            TouchMessage.text = "You haven't touched the object yet";
        }

    }
}

