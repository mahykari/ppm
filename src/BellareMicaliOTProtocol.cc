#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include "BellareMicaliOTProtocol.hh"
#include "MathUtils.hh"
#include "Party.hh"
#include "StringUtils.hh"

template <class P>
BellareMicaliOTProtocol<P>::BellareMicaliOTProtocol(
  QuadraticResidueGroup group,
  P chooser, P sender, P role)
  : Protocol<BellareMicaliOTStage, P>(role),
    group(group),
    chooser(chooser), sender(sender)
{
  this->currentStage = BellareMicaliOTStage::Init;
  this->currentSender = sender;
  if (role == chooser)
    this->chooserData = std::make_unique<BellareMicaliOTChooser>();
  else
    this->senderData = std::make_unique<BellareMicaliOTSender>();
}

template <class P>
void BellareMicaliOTProtocol<P>::updateChooser(bool sigma) {
  assert (this->role == this->chooser and this->chooserData);
  this->chooserData->sigma = sigma;
}

template <class P>
void BellareMicaliOTProtocol<P>::updateSender(
  std::string messages[2])
{
  assert (this->role == this->sender and this->senderData);
  this->senderData->messages[0] = messages[0];
  this->senderData->messages[1] = messages[1];
}

template <class P>
void BellareMicaliOTProtocol<P>::next(std::string message) {
  switch (this->currentStage) {
    case BellareMicaliOTStage::Init:
      if (this->role == this->chooser) {
        // **********************************************************
        // ASSUMPTION: message contains the constant                *
        // encoded as a hexadecimal number.                         *
        // **********************************************************
        auto sendConst = BigInt(message, 16);
        this->chooserData->senderConstant = sendConst;
        printf("I: received sender constant %s\n",
          toString(sendConst).c_str());
      }
      this->currentStage = BellareMicaliOTStage::SendPublicKey;
      this->currentSender = this->chooser;
      break;
    case BellareMicaliOTStage::SendPublicKey:
      // ************************************************************
      // ASSUMPTION: message contains PK0, i.e.,                    *
      // public key for message 0,                                  *
      // encoded as a hexadecimal number.                           *
      // ************************************************************
      if (this->role == this->sender) {
        auto pubKey0 = BigInt(message, 16);
        printf("I: received public key %s\n",
          toString(pubKey0).c_str());
        this->senderData->publicKeys[0] = pubKey0;
        this->senderData->publicKeys[1] = this->group.mul(
          this->senderData->constant,
          this->group.inv(pubKey0));
      }
      this->currentStage =
        BellareMicaliOTStage::SendEncryptedMessages;
      this->currentSender = this->sender;
      break;
    case BellareMicaliOTStage::SendEncryptedMessages:
      if (this->role == this->chooser) {
        // **********************************************************
        // ASSUMPTION: message contains                             *
        // <rand0, enc0>, <rand1, enc1>                             *
        // encoded as four space-separated hexadecimal numbers.     *
        // Both enc0 and enc1 have the same length,                 *
        // which is determined from the protocol group order.       *
        // **********************************************************
        std::string randomStrs[2];
        std::string encryptedMessages[2];
        std::stringstream ss(message);
        ss >> randomStrs[0] >> encryptedMessages[0]
           >> randomStrs[1] >> encryptedMessages[1];
        printf(
          "I: received encrypted messages <%s, %s> <%s, %s>\n",
          randomStrs[0].c_str(), encryptedMessages[0].c_str(),
          randomStrs[1].c_str(), encryptedMessages[1].c_str());
        auto randSigma =
          BigInt(randomStrs[this->chooserData->sigma], 16);
        auto key = toString(
          this->group.exp(
            randSigma, this->chooserData->key), 16);
        auto hashedKey = hashSha512(key);
        auto cipherSigma =
          encryptedMessages[this->chooserData->sigma];
        size_t textLen = cipherSigma.length();
        std::string decryptedMessage(textLen, '0');
        for (size_t i = 0; i < textLen; i++) {
          decryptedMessage[i] = HEX_ALPHABET[
            hexValue(cipherSigma[i]) ^ hexValue(hashedKey[i])];
        }
        printf("I: decrypted message %s\n",
          decryptedMessage.c_str());
      }
      this->currentStage = BellareMicaliOTStage::Done;
    default: break;
  }
}

template <class P>
std::string BellareMicaliOTProtocol<P>::currentMessage() {
  switch (this->currentStage) {
    case BellareMicaliOTStage::Init:
      if (this->role == this->sender) {
        this->senderData->constant = this->group.randomGenerator();
        return toString(this->senderData->constant, 16);
      }
    case BellareMicaliOTStage::SendPublicKey:
      if (this->role == this->chooser) {
        this->chooserData->key = this->group.randomExponent();
        auto pubKeySigma = this->group.exp(
          this->group.baseGenerator, this->chooserData->key);
        BigInt pubKeys[2];
        const bool sigma = this->chooserData->sigma;
        pubKeys[sigma] = this->group.exp(
          this->group.baseGenerator,
          this->chooserData->key);
        pubKeys[not sigma] = this->group.mul(
          this->chooserData->senderConstant,
          this->group.inv(pubKeys[sigma]));
        std::cout
          << "D: public keys are "
          << pubKeys[0] << ' ' << pubKeys[1] << '\n';
        return toString(pubKeys[0], 16);
      }
    case BellareMicaliOTStage::SendEncryptedMessages:
      if (this->role == this->sender) {
        std::string randomStrs[2];
        std::string encryptedMessages[2];
        for (size_t i = 0; i < 2; i++) {
          auto randomExponent = this->group.randomExponent();
          randomStrs[i] = toString(
            this->group.exp(
              this->group.baseGenerator,
              randomExponent), 16);
          auto expdPubKey = toString(
            this->group.exp(
              this->senderData->publicKeys[i],
              randomExponent), 16);
          auto hashedKey = hashSha512(expdPubKey);
          auto message = this->senderData->messages[i];
          size_t textLen = message.length();
          encryptedMessages[i].resize(textLen);
          for (size_t j = 0; j < textLen; j++) {
            encryptedMessages[i][j] = HEX_ALPHABET[
              hexValue(message[j]) ^ hexValue(hashedKey[j])];
          }
        }
        std::string message =
          randomStrs[0] + " " + encryptedMessages[0]
          + " " + randomStrs[1] + " " + encryptedMessages[1];
        printf("I: sending message %s\n", message.c_str());
        return message;
      }
    default: return "";
  }
}

template <class P>
bool BellareMicaliOTProtocol<P>::isOver() {
  return this->currentStage == BellareMicaliOTStage::Done;
}

template class BellareMicaliOTProtocol<MonitoringComponent>;
