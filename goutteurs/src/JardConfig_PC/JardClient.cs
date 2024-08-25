using System;
using System.Collections.Generic;
using System.Text;
using EasyModbus;

namespace JardConfig
{
    public class JardClient
    {
        private ModbusClient m_client;

        public bool Test()
        {
            try 
            {
                m_client.Connect();

                int []res=m_client.ReadInputRegisters(200,3);
                int version=res[0];
                int serial=res[1];
                int soft=res[2];

                if (version!=1)
                    return false;

                m_client.Disconnect();

                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
            finally
            {
                try { m_client.Disconnect(); } catch (Exception) { }
            }
        }

        public JardClient(string sComPort,int iSpeed=115200,byte unit=4)
        {
            m_client = new ModbusClient(sComPort);
            m_client.Baudrate=iSpeed;
            m_client.Parity=System.IO.Ports.Parity.None;
            m_client.StopBits=System.IO.Ports.StopBits.One;
            m_client.NumberOfRetries=2;
            m_client.UnitIdentifier=unit;
            m_client.ConnectionTimeout=100;
        }

        private void resb2pmp_state(JardPumpInfo pmpInf,bool[]resb)
        {
            pmpInf.forced=resb[0];
            pmpInf.enabled=resb[1];
            pmpInf.auto=resb[2];
            pmpInf.remote=resb[3];
            pmpInf.cmd_remote=resb[4];
        }

        private void res2pmp_sched(JardPumpInfo pmpInf,int[]res)
        {
            pmpInf.hour=res[0];
            pmpInf.minute=res[1];
            pmpInf.duration=res[2];
            pmpInf.days=res[3];
            //pmpInf.timer=res[4];
        }

        public void SetDate(DateTime date)
        {
            try 
            {
                m_client.Connect();

                int[]vals=new int[6];
                vals[0]=date.Year;
                vals[1]=date.Month;
                vals[2]=date.Day;
                vals[3]=date.Hour;
                vals[4]=date.Minute;
                vals[5]=date.Second;

                m_client.WriteMultipleRegisters(0,vals);

                m_client.Disconnect();
            }
            catch (Exception ex)
            {
                throw new Exception(ex.Message);
            }
            finally
            {
                try { m_client.Disconnect(); } catch (Exception) { }
            }
        }

        public void SetPump(int num,JardPumpInfo pmpInfo)
        {
            try 
            {
                m_client.Connect();

                int addr;
                if (num==1)
                    addr=10;
                else if (num==2)
                    addr=20;
                else
                    throw new Exception("Numéro de pompe incorrect!");

                bool[]flgs=new bool[5];
                flgs[0]=pmpInfo.forced;
                flgs[1]=pmpInfo.enabled;
                flgs[2]=pmpInfo.auto;
                flgs[3]=pmpInfo.remote;
                flgs[4]=pmpInfo.cmd_remote;
                m_client.WriteMultipleCoils(addr,flgs);

                int[]vals=new int[4];
                vals[0]=pmpInfo.hour;
                vals[1]=pmpInfo.minute;
                vals[2]=pmpInfo.duration;
                vals[3]=pmpInfo.days;
                //vals[4]=pmpInfo.timer;
                m_client.WriteMultipleRegisters(addr,vals);

                m_client.Disconnect();
            }
            catch (Exception ex)
            {
                throw new Exception(ex.Message);
            }
            finally
            {
                try { m_client.Disconnect(); } catch (Exception) { }
            }
        }

        public JardInfo ReadAllFromSlave()
        {            
            try 
            {
                JardInfo info=new JardInfo();
                m_client.Connect();

                int[] res=m_client.ReadHoldingRegisters(0,6);
                int year=res[0];
                int month=res[1];
                int day=res[2];
                int hour=res[3];
                int minute=res[4];
                int second=res[5];
                try
                { 
                    info.date=new  DateTime(year,month,day,hour,minute,second);
                }
                catch (Exception)
                {
                    info.date=DateTime.MinValue;
                }

                res=m_client.ReadInputRegisters(0,6);
                info.batt_dxV=res[0];
                info.sun_dxV=res[1];
                info.temp_dg=res[2];
                info.hum_pc=res[3];
                info.tmrComm=res[4];
                info.tmrVeille=res[5];

                res=m_client.ReadInputRegisters(10,1);
                info.pmp1.timer=res[0];

                res=m_client.ReadInputRegisters(20,1);
                info.pmp2.timer=res[0];

                res=m_client.ReadInputRegisters(100,6);
                info.statTotBoots=res[0];
                info.statTotP1=res[1];
                info.statTotP2=res[2];
                info.statTotSun=res[3];
                info.statTotBtn1=res[4];
                info.statTotBtn2=res[5];

                res=m_client.ReadInputRegisters(200,3);
                info.version=res[0];
                info.serial=res[1];
                info.soft=res[2];

                bool[] resb=m_client.ReadCoils(10,5);
                resb2pmp_state(info.pmp1,resb);

                resb=m_client.ReadCoils(20,5);
                resb2pmp_state(info.pmp2,resb);

                resb=m_client.ReadDiscreteInputs(0,5);
                info.pmp1.led=resb[0];
                info.pmp2.led=resb[1];
                info.batOK=resb[2];
                info.sunOK=resb[3];
                info.veille=resb[4];

                res=m_client.ReadHoldingRegisters(10,4);
                res2pmp_sched(info.pmp1,res);
                res=m_client.ReadHoldingRegisters(20,4);
                res2pmp_sched(info.pmp2,res);

                m_client.Disconnect();

                return info;
            }
            catch (Exception ex)
            {
                throw new Exception(ex.Message);
            }
            finally
            {
                try { m_client.Disconnect(); } catch (Exception) { }
            }
        }
    }
}