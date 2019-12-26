using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Xml;

namespace DB_Test
{
    public class APM_DB
    {
        //HttpWebRequest wReq = null;
        //HttpWebResponse wRes = null;
        String url;
        String dbName;
        XmlDocument xdoc = null;
        XmlNodeList rows = null;


        public APM_DB(String url, String dbName)
        {
            this.url = url.Trim();
            this.dbName = dbName;
            xdoc = new XmlDocument();

        }//생성자

        //--------------------------------------------------------
        private String makeXMLString(String sqlStr)
        {
            String trimedSqlStr = sqlStr.Trim();

            int index = trimedSqlStr.IndexOf(' ');

            String action = trimedSqlStr.Substring(0, index);

            return "<dbRequest>" +
                        "<sql db='" + dbName + "' action='" + action + "'>" +
                            "<![CDATA[" + trimedSqlStr.Substring(index) + "]]>" +
                        "</sql>" +
                    "</dbRequest>";
        }

        //--------------------------------------------------------
        public string query(String sqlStr)
        {
            try
            {
                HttpWebResponse wRes = null;
                HttpWebRequest wReq = (HttpWebRequest)WebRequest.Create(new Uri(url));
                wReq.Method = "POST"; // 전송 방법 "GET" or "POST"

                byte[] byteArray = Encoding.UTF8.GetBytes(makeXMLString(sqlStr));

                Stream dataStream = wReq.GetRequestStream();
                dataStream.Write(byteArray, 0, byteArray.Length);
                dataStream.Close();

                using (wRes = (HttpWebResponse)wReq.GetResponse())
                {
                    Stream respPostStream = wRes.GetResponseStream();
                    StreamReader readerPost = new StreamReader(respPostStream, Encoding.GetEncoding("utf-8"), true);

                    try
                    {
                        //String resStr = readerPost.ReadToEnd();
                        //Console.WriteLine(resStr);
                        //xdoc.LoadXml(resStr);

                        xdoc.LoadXml(readerPost.ReadToEnd());

                        if (xdoc.DocumentElement.GetAttribute("result").Equals("error"))
                        {
                            throw new Exception(string.Format("Query Error:\n{0}", xdoc.DocumentElement.InnerText));

                        }
                        //Console.WriteLine(xdoc.DocumentElement.InnerXml);
                        rows = xdoc.DocumentElement.GetElementsByTagName("row");

                    }
                    catch (Exception ex)
                    {
                        return ("XML Load and Check:\n" + ex.Message);
                    }

                }//using
            }
            catch (ProtocolViolationException ex)
            {
                Console.WriteLine("ProtocolViolationException: " + ex.Message);
            }
            catch (NotSupportedException ex)
            {
                Console.WriteLine("NotSupportedException: " + ex.Message);
            }
            catch (WebException ex)
            {
                if (ex.Status == WebExceptionStatus.ProtocolError && ex.Response != null)
                {
                    var resp = (HttpWebResponse)ex.Response;
                    if (resp.StatusCode == HttpStatusCode.NotFound)
                    {
                        return ("query: WebException Protocol Error: NotFound Error\n" + ex.Message);
                    }
                    else
                    {
                        return (string.Format("query: WebException Protocol Error: 기타 Error: ({0}) \n {1}", resp.StatusCode, ex.Message));
                    }

                }
                else
                {
                    return ("query: WebException: Non Protocol Error\n" + ex.Message);

                }
            }//catch
            catch (Exception ex)
            {
                return ("query: 알 수 없는 Error\n" + ex.Message);

            }

            return null;

        }//query;

        //-------------------------------
        public int readCount
        {
            get
            {
                return rows.Count;
            }

        }

        //---------------------------------------------
        public string getValue(string elemName, int indx = 0)
        {

            try
            {
                return rows[indx].SelectSingleNode(elemName).FirstChild.Value;
            }
            catch (Exception)
            {
                //Console.WriteLine("getValue exception: {0}, index: {1}\n{2}", elemName, indx, rows[indx].InnerXml);
                return "null";
            }
        }


    }//class
}//ns 
