//
// Generated file, do not edit! Created by nedtool 5.6 from MyFrame.msg.
//

#ifndef __MYFRAME_M_H
#define __MYFRAME_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
    #include <bitset>
    typedef std::bitset<8> bits;
// }}

/**
 * Class generated from <tt>MyFrame.msg:26</tt> by nedtool.
 * <pre>
 * packet MyFrame
 * {
 *     \@customize(true);  // see the generated C++ header for more info
 *     int seqNum;
 *     string payload;
 *     int frameType; //0:Data, 1: ACK, 2:NACK
 *     int ackNackNumber;
 *     bits parity;
 * }
 * </pre>
 *
 * MyFrame_Base is only useful if it gets subclassed, and MyFrame is derived from it.
 * The minimum code to be written for MyFrame is the following:
 *
 * <pre>
 * class MyFrame : public MyFrame_Base
 * {
 *   private:
 *     void copy(const MyFrame& other) { ... }

 *   public:
 *     MyFrame(const char *name=nullptr, short kind=0) : MyFrame_Base(name,kind) {}
 *     MyFrame(const MyFrame& other) : MyFrame_Base(other) {copy(other);}
 *     MyFrame& operator=(const MyFrame& other) {if (this==&other) return *this; MyFrame_Base::operator=(other); copy(other); return *this;}
 *     virtual MyFrame *dup() const override {return new MyFrame(*this);}
 *     // ADD CODE HERE to redefine and implement pure virtual functions from MyFrame_Base
 * };
 * </pre>
 *
 * The following should go into a .cc (.cpp) file:
 *
 * <pre>
 * Register_Class(MyFrame)
 * </pre>
 */
class MyFrame_Base : public ::omnetpp::cPacket
{
  protected:
    int seqNum;
    ::omnetpp::opp_string payload;
    int frameType;
    int ackNackNumber;
    bits parity;

  private:
    void copy(const MyFrame_Base& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const MyFrame_Base&);
    // make constructors protected to avoid instantiation

    MyFrame_Base(const MyFrame_Base& other);
    // make assignment operator protected to force the user override it
    MyFrame_Base& operator=(const MyFrame_Base& other);

  public:
    MyFrame_Base(const char *name=nullptr, short kind=0);
    virtual ~MyFrame_Base();
    virtual MyFrame_Base *dup() const override {return new MyFrame_Base(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSeqNum() const;
    virtual void setSeqNum(int seqNum);
    virtual const char * getPayload() const;
    virtual void setPayload(const char * payload);
    virtual int getFrameType() const;
    virtual void setFrameType(int frameType);
    virtual int getAckNackNumber() const;
    virtual void setAckNackNumber(int ackNackNumber);
    virtual bits& getParity();
    virtual const bits& getParity() const {return const_cast<MyFrame_Base*>(this)->getParity();}
    virtual void setParity(const bits& parity);
};


#endif // ifndef __MYFRAME_M_H

