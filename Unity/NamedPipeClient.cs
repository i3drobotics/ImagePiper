using UnityEngine;
using System;
using System.IO;
using System.IO.Pipes;
using System.Collections.Generic;

using OpenCVForUnity.CoreModule;
using OpenCVForUnity.ImgcodecsModule;
using OpenCVForUnity.ImgprocModule;
using OpenCVForUnity.UtilsModule;
using OpenCVForUnity.UnityUtils;

public class NamedPipeClient : MonoBehaviour
{
    [SerializeField, TooltipAttribute("Size of pipe packet (bytes)")]
    public int packet_size = 917504;

    NamedPipeClientStream pipeClient = new NamedPipeClientStream(".", "i3dr_message_pipe", PipeDirection.In, PipeOptions.Asynchronous);
    StreamReader streamReader;
    //Thread readThread;
    string message;

    [SerializeField, TooltipAttribute("Image width")]
    public int image_width = 752;
    [SerializeField, TooltipAttribute("Image height")]
    public int image_height = 480;

    /// <summary>
    /// The right image texture.
    /// </summary>
    [SerializeField, TooltipAttribute("Material to render image")]
    public Material ImageMaterial;

    /// <summary>
    /// The left image texture.
    /// </summary>
    private Texture2D ImageTexture;

    void Start()
    {
        if (ImageTexture == null || ImageTexture.width != image_width || ImageTexture.height != image_height)
            ImageTexture = new Texture2D(image_width, image_height, TextureFormat.RGBA32, false);

        ImageMaterial.mainTexture = ImageTexture;

        Debug.Log("Connecting to pipe...\n");
        pipeClient.Connect();
        streamReader = new StreamReader(pipeClient);
        message = "";
    }

    void Update()
    {
        getPipedData();
    }

    private void getPipedData()
    {
        char[] buf = new char[packet_size];
        //Debug.Log("Reading block");
        int len = streamReader.ReadBlock(buf, 0, packet_size);
        if (len > 0)
        {
            message += new string(buf);
            if (message.Contains("\n"))
            {
                Debug.Log("Full message complete");
                //Debug.Log(message);
                //Mat image = str2mat(message);
                //Debug.LogFormat("Image: ({0}, {1}) Type: {2}", image.width(), image.height(), image.type());
                //UpdateTexture(image);
                message = "";
            }
        } else
        {
            //Debug.Log("No bytes read");
        }
    }

    private Mat str2mat(String s)
    {
        // Decode data
        byte[] byteArr = Convert.FromBase64String(s);
        List<byte> byteList = new List<Byte>(byteArr);
        Mat data = Converters.vector_char_to_Mat(byteList);
        Mat img = Imgcodecs.imdecode(data, Imgcodecs.IMREAD_UNCHANGED);
        return img;
    }

    private void UpdateTexture(Mat image)
    {
        if (image.width() > 0 && image.height() > 0)
        {
            Core.flip(image, image, 0);
            Core.flip(image, image, 1);
            if (image.type() == CvType.CV_8UC3)
            {
                Imgproc.cvtColor(image, image, Imgproc.COLOR_BGR2RGB);
                Utils.matToTexture2D(image, ImageTexture);
            }
            else if (image.type() == CvType.CV_32FC1)
            {
                Mat disp_colormap = DisparityController.Disparity2Colormap(image, 255, false);
                Imgproc.cvtColor(disp_colormap, disp_colormap, Imgproc.COLOR_BGR2RGB);
                Utils.matToTexture2D(disp_colormap, ImageTexture);
            }
            else
            {
                Debug.Log("Unsupported CV image type: " + image.type().ToString());
            }
        }
    }



    /*
    using (NamedPipeClientStream pipeClient =
        new NamedPipeClientStream(".", "i3dr_message_pipe", PipeDirection.In))
    {

        // Connect to the pipe or wait until the pipe is available.
        Debug.Log("Attempting to connect to pipe...");
        pipeClient.Connect();

        Debug.Log("Connected to pipe.");
        //Debug.Log("There are currently {0} pipe server instances open.",
        //   pipeClient.NumberOfServerInstances);
        using (StreamReader sr = new StreamReader(pipeClient))
        {
            // Display the read text to the console
            string temp;
            while ((temp = sr.ReadLine()) != null)
            {
                Debug.Log(temp);
                //Debug.Log("Received from server: {0}", temp);
            }
        }
    }
    Debug.Log("Press Enter to continue...");
    */
}