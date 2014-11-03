/*
 ****************************************************************************
 * user.h
 *
 * Created on: 2008-10-16
 *     Author: 贺辉
 *    License: LGPL
 *    Comment:
 *
 *
 *    =============================  Usage  =============================
 *|
 *|
 *    ===================================================================
 *
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 ****************************************************************************
 */

/*
 ***************************************************************************
 * Last Modified on: 2010-07-07
 * Last Modified by: 贺辉
 ***************************************************************************
 */

#ifndef USER_H_
#define USER_H_

#include <QObject>
#include <QString>
#include <QMetaObject>
#include <QCoreApplication>
#include <QDateTime>

#include "userbase.h"

#include "core_lib.h"

namespace HEHUI {

class CORE_LIB_API User : public UserBase
{
    Q_OBJECT

public:
    enum Gender{GENDER_UNKNOWN = 0, MALE = 1, FEMALE = 2 };
    User(const QString &userID = "", const QString &userName = "", const QString &password = "", QObject *parent = 0);
    virtual ~User();

    bool isRootMode() const {return rootMode;}
    void setRootMode(bool rootMode) {this->rootMode = rootMode;}



    QString getAnswerForSecurity() const
    {
        return answerForSecurity;
    }
    
    QString getEmailForSecurity() const{
        return emailForSecurity;
    }

    QString getBusinessAddress() const
    {
        return businessAddress;
    }

    QString getBusinessEmailAddress() const
    {
        return businessEmailAddress;
    }

    QString getBusinessFaxNumber() const
    {
        return businessFaxNumber;
    }

    QString getBusinessHomepage() const
    {
        return businessHomepage;
    }

    QString getBusinessPhoneNumber() const
    {
        return businessPhoneNumber;
    }

    QString getBusinessZipCode() const
    {
        return businessZipCode;
    }

    QString getCompanyName() const
    {
        return companyName;
    }

    Gender getGender() const
    {
        return gender;
    }
    
    quint8 getAge() const
    {
        return age;
    }

    QDate getBirthday() const{
        return birthday;
    }

    QString getHomeAddress() const
    {
        return homeAddress;
    }

    QString getHomePhoneNumber() const
    {
        return homePhoneNumber;
    }

    QString getHomeZipCode() const
    {
        return homeZipCode;
    }

    QString getFace() const
    {
        return face;
    }

    QString getJobTitle() const
    {
        return jobTitle;
    }

    QString getLastLoginExternalHostAddress() const
    {
        return lastLoginExternalHostAddress;
    }

    quint16 getLastLoginExternalHostPort() const
    {
        return lastLoginExternalHostPort;
    }

    QDateTime getLastLoginTime() const
    {
        return lastLoginTime;
    }

    QDateTime getLastLogoutTime() const
    {
        return lastLogoutTime;
    }

    int getLoginTimes() const
    {
        return loginTimes;
    }

    QString getNickName() const
    {
        if(nickName.isEmpty()){
            return getUserID();
        }
        return nickName;
    }

    QString getPersonalEmailAddress() const
    {
        return personalEmailAddress;
    }

    QString getPersonalHomepage() const
    {
        return personalHomepage;
    }

    QString getQuestionForSecurity() const
    {
        return questionForSecurity;
    }

    QDateTime getRegistrationTime() const
    {
        return registrationTime;
    }

    bool getRootMode() const
    {
        return rootMode;
    }

    QString getTrueName() const
    {
        return trueName;
    }

    QString getdescription() const{
        return this->description;
    }

    void setAnswerForSecurity(const QString &answerForSecurity)
    {
        this->answerForSecurity = answerForSecurity;
    }
    
    void setEmailForSecurity(const QString & emailForSecurity){
        this->emailForSecurity = emailForSecurity;
    }

    void setBusinessAddress(const QString & businessAddress)
    {
        this->businessAddress = businessAddress;
    }

    void setBusinessEmailAddress(const QString & businessEmailAddress)
    {
        this->businessEmailAddress = businessEmailAddress;
    }

    void setBusinessFaxNumber(const QString & businessFaxNumber)
    {
        this->businessFaxNumber = businessFaxNumber;
    }

    void setBusinessHomepage(const QString & businessHomepage)
    {
        this->businessHomepage = businessHomepage;
    }

    void setBusinessPhoneNumber(const QString & businessPhoneNumber)
    {
        this->businessPhoneNumber = businessPhoneNumber;
    }

    void setBusinessZipCode(const QString & businessZipCode)
    {
        this->businessZipCode = businessZipCode;
    }

    void setCompanyName(const QString & companyName)
    {
        this->companyName = companyName;
    }

    void setGender(Gender gender)
    {
        this->gender = gender;
    }
    
    void setAge(quint8 age)
    {
        this->age = age;
    }

    void setBirthday(const QDate &date){
        this->birthday = date;
    }

    void setHomeAddress(const QString & homeAddress)
    {
        this->homeAddress = homeAddress;
    }

    void setHomePhoneNumber(const QString & homePhoneNumber)
    {
        this->homePhoneNumber = homePhoneNumber;
    }

    void setHomeZipCode(const QString & homeZipCode)
    {
        this->homeZipCode = homeZipCode;
    }

    void setFace(const QString &face)
    {
        this->face = face;
    }

    void setJobTitle(const QString & jobTitle)
    {
        this->jobTitle = jobTitle;
    }

    void setLastLoginExternalHostAddress(const QString & lastLoginHostAddress)
    {
        this->lastLoginExternalHostAddress = lastLoginHostAddress;
    }

    void setLastLoginExternalHostPort(quint16 lastLoginHostPort)
    {
        this->lastLoginExternalHostPort = lastLoginHostPort;
    }

    void setLastLoginTime(const QDateTime &lastLoginTime)
    {
        this->lastLoginTime = lastLoginTime;
    }
    void setLastLogoutTime(const QDateTime &lastLogoutTime)
    {
        this->lastLogoutTime = lastLogoutTime;
    }

    void setLoginTimes(int loginTimes)
    {
        this->loginTimes = loginTimes;
    }

    void setNickName(const QString & nickName)
    {
        this->nickName = nickName;
    }

    void setPersonalEmailAddress(const QString & personalEmailAddress)
    {
        this->personalEmailAddress = personalEmailAddress;
    }

    void setPersonalHomepage(const QString & personalHomepage)
    {
        this->personalHomepage = personalHomepage;
    }

    void setQuestionForSecurity(const QString & questionForSecurity)
    {
        this->questionForSecurity = questionForSecurity;
    }

    void setRegistrationTime(const QDateTime &registrationTime)
    {
        this->registrationTime = registrationTime;
    }

    void setTrueName(const QString & trueName)
    {
        this->trueName = trueName;
    }

    void setDescription(const QString & description){
        this->description = description;
    }





private:
    //还原模式，不需要密码//Restore Mode, No Password Required
    bool rootMode;

    QString trueName;
    QString nickName;
    Gender gender;
    quint8 age;
    QDate birthday;
    QString face;

    QString homeAddress;
    QString homePhoneNumber;
    QString homeZipCode;
    QString personalHomepage;
    QString personalEmailAddress;

    QDateTime lastLoginTime;
    QDateTime lastLogoutTime;
    QString lastLoginExternalHostAddress;
    quint16 lastLoginExternalHostPort;

    QString questionForSecurity;
    QString answerForSecurity;
    QString emailForSecurity;


    QString companyName;
    QString jobTitle;
    QString businessAddress;
    QString businessPhoneNumber;
    QString businessZipCode;
    QString businessFaxNumber;
    QString businessHomepage;
    QString businessEmailAddress;

    QDateTime registrationTime;
    int loginTimes;

    QString description;

};

} //namespace HEHUI

#endif /* USER_H_ */
