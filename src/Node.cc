//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "Node.h"
#include <iostream>
#include <fstream>
#include <string>
Define_Module(Node);

std::vector<std::string> readInputFile(const char *filename)
{
    std::ifstream filestream;
    std::string line;
    std::vector<std::string> lines;

    filestream.open(filename, std::ifstream::in);

    if (!filestream)
    {
        throw cRuntimeError("Error opening file '%s'?", filename);
        return lines;
    }
    else
    {
        while (getline(filestream, line))
        {
            lines.push_back(line);
        }
    }
    filestream.close();

    return lines;
}

void Node::initializeInputVectors()
{
    std::vector<std::string> lines = readInputFile("D:/omnetpp-6.0.1/samples/CNProject/input1.txt");
    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it)
    {
        codes.push_back((*it).substr(0, 4));
        messages.push_back((*it).substr(5, (*it).length() - 5));
    }
}

Node::Node()
{
    nextFrameToSend = 0;
    ackExpected = 0;
    frameExpected = 0;
    nbuffered = 0;
}

Node::~Node()
{
    for (int i = 0; i < timeoutEvents.size(); i++)
    {
        cancelEvent(timeoutEvents[i]);
        cancelAndDelete(timeoutEvents[i]);
    }
}

void Node::initialize()
{

    // TODO - Generated method body
    WS = par("WS").intValue();

    if (!strcmp(getName(), "node2"))
    {
        // initialize timeout event and schedule it
        timeout = par("TO").doubleValue();

        // initialize input vectors messages and codes
        initializeInputVectors();

        // check if window size is greater than the number of messages
        if (WS > messages.size())
        {
            WS = messages.size();
        }

        // schedule first window of frames
        for (int i = 0; i < WS; i++)
        {
            EV << "At time [" << simTime() + par("PT").doubleValue() * (i) << "], Node [2] , Introducing channel error with code=[" << codes.at(i) << "]" << endl;

            MyFrame_Base *frame = new MyFrame_Base("");

            frame->setSeqNum(i);
            nextFrameToSend = nextFrameToSend + 1;

            frame->setFrameType(0);
            frame->setPayload(messages.at(i).c_str());

            // nbuffered is the number of frames that are sent but not yet acknowledged
            nbuffered = nbuffered + 1;

            cMessage *timeoutEvent = new cMessage("timeoutEvent");
            timeoutEvents.push_back(timeoutEvent);
            scheduleAt(simTime() + timeout + (par("PT").doubleValue() * (i + 1)), timeoutEvents[i]);

            // if error code is 0000 then no error delay
            if (!strcmp(codes.at(i).c_str(), "0000"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1) + par("TD").doubleValue());
                sendDelayed(frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0], Delay [0]" << endl;
            }
            // calculate total delay all channels have processing delay and transmission delay

            // if error code is 0001 then add error delay
            else if (!strcmp(codes.at(i).c_str(), "0001"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1) + par("TD").doubleValue()) + par("ED").doubleValue();
                sendDelayed(frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0], Delay ["
                   << par("ED").doubleValue() << "]" << endl;
            }

            else if (!strcmp(codes.at(i).c_str(), "0010"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1) + par("TD").doubleValue());
                sendDelayed(frame, totalDelay, "out");

                MyFrame_Base *dup_frame = frame->dup();
                totalDelay += par("DD").doubleValue();
                sendDelayed(dup_frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [1], Delay [0]" << endl;
            }

            else if (!strcmp(codes.at(i).c_str(), "0011"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1) + par("TD").doubleValue()) + par("ED").doubleValue();
                sendDelayed(frame, totalDelay, "out");

                MyFrame_Base *dup_frame = frame->dup();
                totalDelay += par("DD").doubleValue();
                sendDelayed(dup_frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [1], Delay ["
                   << par("ED").doubleValue() << "]" << endl;
            }
        }
    }
}

void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    // TODO:receive msg from coordinator to decide which node will begin

    // starternode is the sender node (will be handled later after implementing coordinator)
    const char *starternode = "node2";

    // if timeout event is received
    // schedule next window of frames

    if (!strcmp(msg->getName(), "timeoutEvent"))
    {
        EV << "Time out event at time " << simTime() << ", at Node[2] for frame with seq_num= " << ackExpected % WS << endl;

        if (abs(nextFrameToSend - ackExpected) == 0)
        {
            EV << "\nEnding Sim ...\n";
            endSimulation();
        }

        nextFrameToSend = ackExpected;

        int nextFrameToSendTemp = nextFrameToSend;
        nbuffered = 0;
        for (int i = nextFrameToSendTemp; i < nextFrameToSendTemp + (WS) && i < messages.size(); i++)
        {
            EV << "At time [" << simTime() + par("PT").doubleValue() * (i) << "], Node [2] , Introducing channel error with code=[" << codes.at(i) << "]" << endl;

            nextFrameToSend = nextFrameToSend + 1;

            MyFrame_Base *frame = new MyFrame_Base("");
            frame->setSeqNum(i % WS);
            frame->setFrameType(0);
            frame->setPayload(messages.at(i).c_str());

            cancelEvent(timeoutEvents[i]);
            scheduleAt(simTime() + timeout + (par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp)), timeoutEvents[i]);
            nbuffered = nbuffered + 1;

            // if error code is 0000 then no error delay
            if (!strcmp(codes.at(i).c_str(), "0000"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) + par("TD").doubleValue());
                sendDelayed(frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0], Delay [0]" << endl;
            }
            // calculate total delay all channels have processing delay and transmission delay

            // if error code is 0001 then add error delay
            else if (!strcmp(codes.at(i).c_str(), "0001"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) + par("TD").doubleValue());
                if (i != nextFrameToSendTemp + 1)
                {
                    totalDelay += par("ED").doubleValue();
                }
                sendDelayed(frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0], Delay ["
                   << par("ED").doubleValue() << "]" << endl;
            }

            else if (!strcmp(codes.at(i).c_str(), "0010"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) + par("TD").doubleValue());
                sendDelayed(frame, totalDelay, "out");

                MyFrame_Base *dup_frame = frame->dup();
                totalDelay += par("DD").doubleValue();
                sendDelayed(dup_frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [1], Delay [0]" << endl;
            }

            else if (!strcmp(codes.at(i).c_str(), "0011"))
            {
                double totalDelay = (par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) + par("TD").doubleValue());
                if (i != nextFrameToSendTemp + 1)
                {
                    totalDelay += par("ED").doubleValue();
                }
                sendDelayed(frame, totalDelay, "out");

                MyFrame_Base *dup_frame = frame->dup();
                totalDelay += par("DD").doubleValue();
                sendDelayed(dup_frame, totalDelay, "out");

                EV << "At time [" << simTime() + par("PT").doubleValue() * (i + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                   << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                   << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [1], Delay ["
                   << par("ED").doubleValue() << "]" << endl;
            }
        }
    }

    // if frame is received
    // check if frame is of type 1
    // if yes then ack is received
    // if no then data is received
    // if ack is received increment ackExpected and decrement nbuffered
    // and cancel timeout event and schedule it again from current time

    else if (!strcmp(getName(), starternode))
    {
        MyFrame_Base *frame = check_and_cast<MyFrame_Base *>(msg);
        if (frame->getFrameType() == 1 && frame->getAckNackNumber() == ackExpected % (WS))
        {
            nbuffered = nbuffered - 1;

            EV << "At time [" << simTime() << "], Node[2] received ack "
               << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
               << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0]" << endl;

            cancelEvent(timeoutEvents[ackExpected]);
            ackExpected = ackExpected + 1;
        }

        if (nbuffered < (WS))
        {
            if (nextFrameToSend < messages.size())
            {
                int nextFrameToSendTemp = nextFrameToSend;
                for (int j = nextFrameToSendTemp; j < nextFrameToSendTemp + (WS - nbuffered) && j < messages.size(); j++)
                {
                    EV << "At time [" << simTime() + par("PT").doubleValue() * (j - nextFrameToSendTemp) << "], Node [2] , Introducing channel error with code=[" << codes.at(j) << "]" << endl;

                    MyFrame_Base *dup_frame = frame->dup();
                    dup_frame->setSeqNum(j % WS);
                    dup_frame->setFrameType(0);
                    dup_frame->setPayload(messages.at(j).c_str());

                    cMessage *timeoutEvent = new cMessage("timeoutEvent");
                    timeoutEvents.push_back(timeoutEvent);
                    scheduleAt(simTime() + timeout + (par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp)), timeoutEvents[j]);

                    nextFrameToSend = nextFrameToSend + 1;
                    nbuffered = nbuffered + 1;

                    // if error code is 0000 then no error delay
                    if (!strcmp(codes.at(j).c_str(), "0000"))
                    {
                        double totalDelay = (par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) + par("TD").doubleValue());
                        sendDelayed(dup_frame, totalDelay, "out");

                        EV << "At time [" << simTime() + par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                           << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                           << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0], Delay [0]" << endl;
                    }
                    // calculate total delay all channels have processing delay and transmission delay

                    // if error code is 0001 then add error delay
                    else if (!strcmp(codes.at(j).c_str(), "0001"))
                    {
                        double totalDelay = (par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) + par("TD").doubleValue());

                        totalDelay += par("ED").doubleValue();

                        sendDelayed(dup_frame, totalDelay, "out");

                        EV << "At time [" << simTime() + par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                           << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                           << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [0], Delay ["
                           << par("ED").doubleValue() << "]" << endl;
                    }

                    else if (!strcmp(codes.at(j).c_str(), "0010"))
                    {
                        double totalDelay = (par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) + par("TD").doubleValue());
                        sendDelayed(dup_frame, totalDelay, "out");

                        MyFrame_Base *dup_frame2 = frame->dup();
                        totalDelay += par("DD").doubleValue();
                        sendDelayed(dup_frame2, totalDelay, "out");

                        EV << "At time [" << simTime() + par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                           << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                           << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [1], Delay [0]" << endl;
                    }

                    else if (!strcmp(codes.at(j).c_str(), "0011"))
                    {
                        double totalDelay = (par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) + par("TD").doubleValue());

                        totalDelay += par("ED").doubleValue();

                        sendDelayed(dup_frame, totalDelay, "out");

                        MyFrame_Base *dup_frame2 = frame->dup();
                        totalDelay += par("DD").doubleValue();
                        sendDelayed(dup_frame2, totalDelay, "out");

                        EV << "At time [" << simTime() + par("PT").doubleValue() * (j + 1 - nextFrameToSendTemp) << "], Node[2] sent frame "
                           << "with seq_num=[" << frame->getSeqNum() << "] and payload= [" << frame->getPayload()
                           << "] and trailer= [] , Modified [0] , Lost [No], Duplicate [1], Delay ["
                           << par("ED").doubleValue() << "]" << endl;
                    }
                }
            }
        }
        cancelAndDelete(frame);
    }

    else if (strcmp(getName(), starternode))
    {
        MyFrame_Base *frame = check_and_cast<MyFrame_Base *>(msg);
        if (frame->getFrameType() == 0 && frame->getSeqNum() == frameExpected % (WS))
        {
            frameExpected = (frameExpected + 1);
            MyFrame_Base *reply = frame->dup();
            reply->setFrameType(1);
            reply->setAckNackNumber(frame->getSeqNum());
            EV << "At time [" << simTime() + par("PT").doubleValue() << "], Node[1] Sending ACK with number " << reply->getSeqNum() + 1 << endl;
            sendDelayed(reply, par("PT").doubleValue() + par("TD").doubleValue(), "out");
        }

        cancelAndDelete(frame);
    }
}
