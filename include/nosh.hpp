/*
 * simptst.hpp
 *
 *  Created on: 18 aug. 2016
 *      Author: SERAGUD
 */

#ifndef NOSH_NOSH_HPP_
#define NOSH_NOSH_HPP_

#include <iostream>
#include <iterator>
#include <vector>
#include <array>
#include <stdint.h>
#include <memory>
#include <map>
#include <fstream>

using namespace std;

typedef unsigned int uint_t;

class AssertException {

};

template<typename Type>
class Generator;

template<typename Type>
class Iterator;

template<typename Type>
class Iterator: public std::iterator<std::input_iterator_tag, Type> {
	Generator<Type> const &mGen;

	uint_t mIdx;
public:
	Iterator(Generator<Type> const &aGen, uint_t aIdx) :
			mGen(aGen), mIdx(aIdx) {
	}

	Iterator(const Iterator& aOther) :
			mGen(aOther.mGen), mIdx(aOther.mIdx) {
	}

	Iterator& operator++() {
		++mIdx;
		return *this;
	}

	Iterator operator++(int) {
		Iterator lRet(*this);
		operator++();
		return lRet;
	}

	bool operator==(const Iterator& aRhs) {
		return mIdx == aRhs.mIdx;
	}

	bool operator!=(const Iterator& aRhs) {
		return mIdx != aRhs.mIdx;
	}

	bool operator<(const Iterator& aRhs) {
		return mIdx < aRhs.mIdx;
	}

	Type operator*() {
		return mGen.at(mIdx);
	}
};

template<typename Type>
class Generator {
public:
	virtual Iterator<Type> begin() {
		return Iterator<Type>(*this, size());
	}

	virtual Iterator<Type> end() {
		return Iterator<Type>(*this, 0);
	}

	virtual Iterator<Type> cbegin() const {
		return Iterator<Type>(*this, 0);
	}

	virtual Iterator<Type> cend() const {
		return Iterator<Type>(*this, size());
	}

	virtual uint_t size() const = 0;

	virtual Type at(uint_t const aIdx) const = 0;

	virtual Generator<Type>& clone() const = 0;

	virtual ~Generator() {

	}
};

template<typename Type>
class GeneratorValues: public Generator<Type> {
private:
	vector<Type> const mVec;
public:
	GeneratorValues(vector<Type> const aVec) :
			mVec(aVec) {
	}

	virtual Iterator<Type> begin() {
		return Iterator<Type>(*this, 0);
	}

	virtual Iterator<Type> end() {
		return Iterator<Type>(*this, mVec.size());
	}

	virtual Iterator<Type> cbegin() const {
		return Iterator<Type>(*this, 0);
	}

	virtual Iterator<Type> cend() const {
		return Iterator<Type>(*this, mVec.size());
	}

	virtual uint_t size() const {
		return mVec.size();
	}

	virtual Type at(uint_t const aIdx) const {
		return mVec.at(aIdx);
	}

	virtual ~GeneratorValues() {
	}

	virtual Generator<Type>& clone() const {
		GeneratorValues *lRet = new GeneratorValues(mVec);

		return *lRet;
	}
};

template<typename Type>
class GeneratorRange: public Generator<Type> {
private:
	Type const mStart;

	Type const mEnd;

	Type const mStep;
public:
	GeneratorRange(Type aStart, Type aEnd, Type aStep) :
			mStart(aStart), mEnd(aEnd), mStep(aStep) {
	}

	GeneratorRange(Type aStart, Type aEnd) :
			mStart(aStart), mEnd(aEnd), mStep(1) {
	}

	virtual ~GeneratorRange() {
	}

	Iterator<Type> begin() {
		return Iterator<Type>(*this, 0);
	}

	Iterator<Type> end() {
		return Iterator<Type>(*this, size());
	}

	Iterator<Type> cbegin() const {
		return Iterator<Type>(*this, 0);
	}

	Iterator<Type> cend() const {
		return Iterator<Type>(*this, size());
	}

	uint_t size() const {
		if (mStart > mEnd) {
			throw exception();
		}

		return ((mEnd - mStart) / mStep) + 1;
	}

	Type at(uint_t const aIdx) const {
		auto lPos = mStart + (aIdx * mStep);

		if (lPos < mEnd) {
			return (Type) lPos;
		}

		return mEnd;
	}

	virtual Generator<Type>& clone() const {
		GeneratorRange *lRet = new GeneratorRange(mStart, mEnd, mStep);

		return *lRet;
	}
};

template<typename Type>
GeneratorRange<Type> range(Type aStart, Type aEnd) {
	return GeneratorRange<Type>(aStart, aEnd, 1);
}

template<uint16_t Num>
class CombinatorArray: public Generator<vector<uint_t>> {
private:
	array<uint_t, Num> mLengths;

	uint_t mSize = 1;

	array<uint_t, Num> mVecPos;

public:
	explicit CombinatorArray() {
	}

	explicit CombinatorArray(array<uint_t, Num> aVecs) :
			mLengths(aVecs) {
		if (Num == 0) {
			throw exception();
		}

		uint_t lSize = 1;

		for (uint_t cIdx = 0; cIdx < aVecs.size(); cIdx++) {
			if (mLengths[cIdx] == 0) {
				throw exception();
			}

			lSize *= mLengths[cIdx];
		}

		mSize = lSize;

		mVecPos.fill(0U);
	}

	virtual ~CombinatorArray() {
	}

	Iterator<vector<uint_t>> begin() {
		return Iterator<array<uint_t, Num>>(*this, 0);
	}

	Iterator<vector<uint_t>> end() {
		return Iterator<vector<uint_t>>(*this, size());
	}

	Iterator<vector<uint_t>> cbegin() const {
		return Iterator<vector<uint_t>>(*this, 0);
	}

	Iterator<vector<uint_t>> cend() const {
		return Iterator<vector<uint_t>>(*this, size());
	}

	bool hasNext() {
		return mVecPos.back() < mLengths.back();
	}

	vector<uint_t> current() const {
		vector<uint_t> lRet(mLengths.size());

		if (mVecPos.back() >= mLengths.back()) {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mLengths[cIdx] - 1;
			}
		} else {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mVecPos[cIdx];
			}
		}

		return lRet;
	}

	vector<uint_t> next() {
		vector<uint_t> lRet(mLengths.size());

		if (mVecPos.back() >= mLengths.back()) {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mLengths[cIdx] - 1;
			}
			return lRet;
		} else {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mVecPos[cIdx];
			}
		}

		mVecPos[0]++;

		if (mVecPos[0] >= mLengths[0]) {
			mVecPos[0] = 0;

			for (uint_t cIdx = 1; cIdx < mLengths.size(); cIdx++) {
				mVecPos[cIdx]++;
				if (mVecPos[cIdx] == mLengths[cIdx]) {
					if (cIdx == (mLengths.size() - 1)) {
						break;
					}
					mVecPos[cIdx] = 0;
				} else {
					break;
				}
			}
		}

		return lRet;
	}

	uint_t size() const {
		return mSize;
	}

	vector<uint_t> at(uint_t const aIdx) const {
		vector<uint_t> lRet(mLengths.size());

		if (aIdx >= mSize) {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mLengths[cIdx] - 1;
			}
			return lRet;
		}

		uint_t lRemain = aIdx;
		uint_t lSize = mSize / mLengths[0];

		for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
			lRet[cIdx] = lRemain / lSize;
			lRemain -= lRet[cIdx] * lSize;
			if ((cIdx + 1) < mLengths.size()) {
				lSize /= mLengths[cIdx + 1];
			}
		}

		return lRet;
	}

	virtual Generator& clone() const {
		CombinatorArray *lRet = new CombinatorArray<Num>(mLengths);

		return *lRet;
	}
};

class Combinator: public Generator<vector<uint_t>> {
private:
	vector<uint_t> mLengths;

	uint_t mSize = 1;

	vector<uint_t> mVecPos;

public:
	explicit Combinator() {
	}

	explicit Combinator(vector<uint_t> aVecs) :
			mLengths(aVecs) {
		if (mLengths.size() == 0) {
			throw exception();
		}

		uint_t lSize = 1;

		for (uint_t cIdx = 0; cIdx < aVecs.size(); cIdx++) {
			if (mLengths[cIdx] == 0) {
				throw exception();
			}

			lSize *= mLengths[cIdx];
		}

		mSize = lSize;

		mVecPos = vector<uint_t>(mLengths.size());
	}

	virtual ~Combinator() {
	}

	Iterator<vector<uint_t>> begin() {
		return Iterator<vector<uint_t>>(*this, 0);
	}

	Iterator<vector<uint_t>> end() {
		return Iterator<vector<uint_t>>(*this, size());
	}

	Iterator<vector<uint_t>> cbegin() const {
		return Iterator<vector<uint_t>>(*this, 0);
	}

	Iterator<vector<uint_t>> cend() const {
		return Iterator<vector<uint_t>>(*this, size());
	}

	bool hasNext() {
		return mVecPos.back() < mLengths.back();
	}

	vector<uint_t> current() const {
		vector<uint_t> lRet(mLengths.size());

		if (mVecPos.back() >= mLengths.back()) {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mLengths[cIdx] - 1;
			}
		} else {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mVecPos[cIdx];
			}
		}

		return lRet;
	}

	vector<uint_t> next() {
		vector<uint_t> lRet(mLengths.size());

		if (mVecPos.back() >= mLengths.back()) {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mLengths[cIdx] - 1;
			}
			return lRet;
		} else {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mVecPos[cIdx];
			}
		}

		mVecPos[0]++;

		if (mVecPos[0] >= mLengths[0]) {
			mVecPos[0] = 0;

			for (uint_t cIdx = 1; cIdx < mLengths.size(); cIdx++) {
				mVecPos[cIdx]++;
				if (mVecPos[cIdx] == mLengths[cIdx]) {
					if (cIdx == (mLengths.size() - 1)) {
						break;
					}
					mVecPos[cIdx] = 0;
				} else {
					break;
				}
			}
		}

		return lRet;
	}

	uint_t size() const {
		return mSize;
	}

	vector<uint_t> at(uint_t const aIdx) const {
		vector<uint_t> lRet(mLengths.size());

		if (aIdx >= mSize) {
			for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
				lRet[cIdx] = mLengths[cIdx] - 1;
			}
			return lRet;
		}

		uint_t lRemain = aIdx;
		uint_t lSize = mSize / mLengths[0];

		for (uint_t cIdx = 0; cIdx < mLengths.size(); cIdx++) {
			lRet[cIdx] = lRemain / lSize;
			lRemain -= lRet[cIdx] * lSize;
			if ((cIdx + 1) < mLengths.size()) {
				lSize /= mLengths[cIdx + 1];
			}
		}

		return lRet;
	}

	virtual Generator& clone() const {
		Combinator *lRet = new Combinator(mLengths);

		return *lRet;
	}
};

class Param {
public:
	virtual ~Param() {
	}

	virtual string printableName() const = 0;
};

class LineInfo {
	char const * const mFilename;

	uint_t const mLineNum;
public:
	LineInfo(char const * const aFilename, uint_t const aLineNum) :
			mFilename(aFilename), mLineNum(aLineNum) {
	}

	const char * filename(void) const {
		return mFilename;
	}

	uint_t lineNum(void) const {
		return mLineNum;
	}
};

#define kLineInfo LineInfo(__FILE__, __LINE__)

class Test {
	bool mTstFailed = 0;

	bool mShowAssertionMsg = true;

	virtual void setShowAssertionMsg(bool const aVal) final {
		mShowAssertionMsg = aVal;
	}

	friend class TestRunnerInterface;

public:
	virtual ~Test() {
	}

	virtual void setup() {
	}

	virtual void teardown() {
	}

	virtual void run() = 0;

	virtual string testName(void) = 0;

	virtual string printableParam(void) {
		return "";
	}

	virtual string printableBreif(void) {
		return "";
	}

	bool didFail(void) {
		return mTstFailed;
	}

	template<typename Type>
	bool expectEqual(Type const aLhs, Type const aRhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return true;
		}

		mTstFailed = true;

		if (mShowAssertionMsg) {
			cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
					<< " => Test case failed!" << endl;
			cout << "\t Reason => " << aLhs << " != " << aRhs << endl;
			if (!aContext.empty()) {
				cout << "\t Context => ";
				for (auto cContext : aContext) {
					cout << cContext.first << ": " << cContext.second << endl;
				}
			}
		}

		return false;
	}

	template<typename Type>
	bool expectEqual(Type const aLhs, Type const aRhs, LineInfo const & aLine) {
		return expectEqual(aLhs, aRhs, aLine, { });
	}

	template<typename Type>
	void assertEqual(Type const aLhs, Type const aRhs, LineInfo const & aLine) {
		if (!expectEqual<Type>(aLhs, aRhs, aLine)) {
			throw AssertException();
		}
	}

	bool expectEqualUInt8(uint8_t const aLhs, uint8_t const aRhs,
			LineInfo const & aLine, map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return true;
		}

		mTstFailed = true;

		if (mShowAssertionMsg) {
			cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
					<< " => Test case failed!" << endl;
			cout << "\t Reason => " << (uint16_t) aLhs << " != " << (uint16_t) aRhs
					<< endl;
			if (!aContext.empty()) {
				cout << "\t Context => ";
				for (auto cContext : aContext) {
					cout << cContext.first << ": " << cContext.second << endl;
				}
			}
		}

		return false;
	}

	bool expectEqualUInt8(uint8_t const aLhs, uint8_t const aRhs,
			LineInfo const & aLine) {
		return expectEqualUInt8(aLhs, aRhs, aLine, { });
	}

	void assertEqualUInt8(uint8_t const aLhs, uint8_t const aRhs,
			LineInfo const & aLine) {
		if (!expectEqualUInt8(aLhs, aRhs, aLine)) {
			throw AssertException();
		}
	}

	void assertEqualUInt8(uint8_t const aLhs, uint8_t const aRhs,
			LineInfo const & aLine, map<string, string> const &aContext) {
		if (!expectEqualUInt8(aLhs, aRhs, aLine, aContext)) {
			throw AssertException();
		}
	}

	bool expectEqualUInt16(uint16_t const aLhs, uint16_t const aRhs,
			LineInfo const & aLine, map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return true;
		}

		mTstFailed = true;

		if (mShowAssertionMsg) {
			cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
					<< " => Test case failed!" << endl;
			cout << "\t Reason => " << (uint16_t) aLhs << " != " << (uint16_t) aRhs
					<< endl;
			if (!aContext.empty()) {
				cout << "\t Context => ";
				for (auto cContext : aContext) {
					cout << cContext.first << ": " << cContext.second << endl;
				}
			}
		}

		return false;
	}

	bool expectEqualUInt16(uint16_t const aLhs, uint16_t const aRhs,
			LineInfo const & aLine) {
		return expectEqualUInt16(aLhs, aRhs, aLine, { });
	}

	void assertEqualUInt16(uint16_t const aLhs, uint16_t const aRhs,
			LineInfo const & aLine, map<string, string> const &aContext) {
		if (!expectEqualUInt16(aLhs, aRhs, aLine, aContext)) {
			throw AssertException();
		}
	}

	void assertEqualUInt16(uint16_t const aLhs, uint16_t const aRhs,
			LineInfo const & aLine) {
		if (!expectEqualUInt16(aLhs, aRhs, aLine)) {
			throw AssertException();
		}
	}

	bool expectEqualUInt32(uint32_t const aLhs, uint32_t const aRhs,
			LineInfo const & aLine, map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return true;
		}

		mTstFailed = true;

		if (mShowAssertionMsg) {
			cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
					<< " => Test case failed!" << endl;
			cout << "\t Reason => " << (uint32_t) aLhs << " != " << (uint32_t) aRhs
					<< endl;
			if (!aContext.empty()) {
				cout << "\t Context => ";
				for (auto cContext : aContext) {
					cout << cContext.first << ": " << cContext.second << endl;
				}
			}
		}

		return false;
	}

	bool expectEqualUInt32(uint32_t const aLhs, uint32_t const aRhs,
			LineInfo const & aLine) {
		return expectEqualUInt32(aLhs, aRhs, aLine, { });
	}

	void assertEqualUInt32(uint32_t const aLhs, uint32_t const aRhs,
			LineInfo const & aLine, map<string, string> const &aContext = { }) {
		if (!expectEqualUInt32(aLhs, aRhs, aLine, aContext)) {
			throw AssertException();
		}
	}

	bool expectTrue(bool const aLhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs != false) {
			return true;
		}

		mTstFailed = true;

		if (mShowAssertionMsg) {
			cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
					<< " => Test case failed!" << endl;
			cout << "\t Reason => " << (bool) aLhs << " not true!" << endl;
			if (!aContext.empty()) {
				cout << "\t Context => ";
				for (auto cContext : aContext) {
					cout << cContext.first << ": " << cContext.second << endl;
				}
			}
		}

		return false;
	}

	bool expectTrue(bool const aLhs, LineInfo const &aLine) {
		return expectTrue(aLhs, aLine, { });
	}

	void assertTrue(bool const aLhs, LineInfo const &aLine,
			map<string, string> const &aContext) {
		if (!expectTrue(aLhs, aLine, aContext)) {
			throw AssertException();
		}
	}

	void assertTrue(bool const aLhs, LineInfo const &aLine) {
		if (!expectTrue(aLhs, aLine, { })) {
			throw AssertException();
		}
	}

	bool expectFalse(bool const aLhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == false) {
			return true;
		}

		mTstFailed = true;

		if (mShowAssertionMsg) {
			cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
					<< " => Test case failed!" << endl;
			cout << "\t Reason => " << (bool) aLhs << " not false!" << endl;
			if (!aContext.empty()) {
				cout << "\t Context => ";
				for (auto cContext : aContext) {
					cout << cContext.first << ": " << cContext.second << endl;
				}
			}
		}

		return false;
	}

	void expectFalse(bool const aLhs, LineInfo const &aLine) {
		expectFalse(aLhs, aLine, { });
	}

	void assertFalse(bool const aLhs, LineInfo const &aLine) {
		if (!expectFalse(aLhs, aLine, { })) {
			throw AssertException();
		}
	}

	void assertFalse(bool const aLhs, LineInfo const &aLine,
			map<string, string> const &aContext) {
		if (!expectFalse(aLhs, aLine, aContext)) {
			throw AssertException();
		}
	}
};

template<typename TypeParam>
class TestWithParam: public Test {
public:
	virtual ~TestWithParam() {

	}
};

enum VerbosityMask {
	VerbosityMaskTestIntro = 0x1,
	VerbosityMaskAssertReason = 0x2,
	VerbosityMaskTestResult = 0x4,
	VerbosityMaskTestResultSuccess = 0x8,
};

class Verbosity {
	uint8_t mVerbosity = VerbosityMaskTestIntro | VerbosityMaskAssertReason
			| VerbosityMaskTestResult;
public:
	Verbosity() {
	}

	Verbosity(uint8_t const aVerbosity) :
			mVerbosity(aVerbosity) {
	}

	bool showTestIntro(void) const {
		return (mVerbosity & VerbosityMaskTestIntro) != 0;
	}

	bool showAssertReason(void) const {
		return (mVerbosity & VerbosityMaskAssertReason) != 0;
	}

	bool showTestResult(void) const {
		return (mVerbosity & VerbosityMaskTestResult) != 0;
	}

	bool showTestResultSuccess(void) const {
		return (mVerbosity & VerbosityMaskTestResultSuccess) != 0;
	}
};

class TestRunnerInterface {
protected:
	void setShowAssertionMsg(Test &aTest, bool const aVerbosity) {
		aTest.setShowAssertionMsg(aVerbosity);
	}

public:
	virtual ~TestRunnerInterface() {

	}

	virtual void exec(Test &aTest) = 0;

	template<class TestWithParam, typename TypeParam>
	void execWithParam(Generator<TypeParam> const &aGen);

	virtual void printSummary(void) = 0;
};

class TestStatus {
public:
	bool status;

	string name;

	string breif;

	bool valid;

	TestStatus() :
			status(false), name(""), breif(""), valid(false) {
	}

	TestStatus(const bool status, const string name, const string breif,
			const bool valid = true) :
			status(status), name(name), breif(breif), valid(valid) {
	}
};

class TestRunner: public TestRunnerInterface {
	Verbosity mVerbosity;

	uint64_t mTests = 0;

	uint64_t mTestsSucceeded = 0;

	map<string, TestStatus> mExecuted;

public:
	TestRunner(Verbosity const aVerbosity = Verbosity()) :
			mVerbosity(aVerbosity) {
	}

	~TestRunner() {
	}

	void exec(Test &aTest) {
		string printable = aTest.testName();
		string paramStr = aTest.printableParam();
		if (paramStr.length() != 0) {
			printable += " with " + paramStr;
		}

		mTests++;
		if (mVerbosity.showTestIntro()) {
			cout << "Running " << printable << " ..." << endl;
		}

		TestRunnerInterface::setShowAssertionMsg(aTest,
				mVerbosity.showAssertReason());

		aTest.setup();

		try {
			aTest.run();
		} catch (AssertException &exc) {
			//Should we show the assert here?
		}

		aTest.teardown();

		bool repSts = true;
		if (mExecuted[aTest.testName()].valid == true) {
			repSts = mExecuted[aTest.testName()].status;
		}
		repSts &= !aTest.didFail();
		mExecuted[aTest.testName()] = TestStatus(repSts, aTest.testName(),
				aTest.printableBreif());

		if (aTest.didFail() == false) {
			mTestsSucceeded++;
		}

		if (mVerbosity.showTestResult()) {
			if (mVerbosity.showTestIntro()) {
				if (aTest.didFail()) {
					cout << "Test failed!" << endl;
				} else {
					if (mVerbosity.showTestResultSuccess()) {
						cout << "Test succeeded!" << endl;
					}
				}
			} else {
				if (aTest.didFail()) {
					cout << "Test failed: " << printable << "!" << endl;
				} else {
					if (mVerbosity.showTestResultSuccess()) {
						cout << "Test succeeded!" << endl;
					}
				}
			}
		}
	}

	template<class TestWithParam, typename ParamType>
	void execWithUint8(ParamType const aParam) {
		mTests++;
		auto bTest = TestWithParam(aParam);

		if (mVerbosity.showTestIntro()) {
			cout << "Running " << bTest.testName() << " with " << aParam << " ..."
					<< endl;
		}

		TestRunnerInterface::setShowAssertionMsg(bTest,
				mVerbosity.showAssertReason());

		bTest.setup();

		try {
			bTest.run();
		} catch (AssertException &exc) {
			//Should we show the assert here?
		}

		bTest.teardown();

		bool repSts = true;
		if (mExecuted[bTest.testName()].valid == true) {
			repSts = mExecuted[bTest.testName()].status;
		}
		repSts &= !bTest.didFail();
		mExecuted[bTest.testName()] = TestStatus(repSts, bTest.testName(),
				bTest.printableBreif());

		if (bTest.didFail() == false) {
			mTestsSucceeded++;
		}

		if (mVerbosity.showTestResult()) {
			if (mVerbosity.showTestIntro()) {
				if (bTest.didFail()) {
					cout << "Test failed!" << endl;
				} else {
					if (mVerbosity.showTestResultSuccess()) {
						cout << "Test succeeded!" << endl;
					}
				}
			} else {
				if (bTest.didFail()) {
					cout << "Test failed: " << bTest.testName() << " with " << aParam
							<< "!" << endl;
				} else {
					if (mVerbosity.showTestResultSuccess()) {
						cout << "Test succeeded!" << endl;
					}
				}
			}
		}
	}

	template<class TestWithParam, typename ParamType>
	void execWithVector(vector<ParamType> const aVector) {
		for (auto cParam = aVector.cbegin(); cParam < aVector.cend(); cParam++) {
			mTests++;
			auto bTest = TestWithParam(*cParam);

			string printable = bTest.testName();
			string paramStr = bTest.printableParam();
			if (paramStr.length() != 0) {
				printable += " with " + paramStr;
			}

			if (mVerbosity.showTestIntro()) {
				cout << "Running " << printable << " ..." << endl;
			}

			TestRunnerInterface::setShowAssertionMsg(bTest,
					mVerbosity.showAssertReason());

			bTest.setup();

			try {
				bTest.run();
			} catch (AssertException &exc) {
				//Should we show the assert here?
			}

			bTest.teardown();

			bool repSts = true;
			if (mExecuted[bTest.testName()].valid == true) {
				repSts = mExecuted[bTest.testName()].status;
			}
			repSts &= !bTest.didFail();
			mExecuted[bTest.testName()] = TestStatus(repSts, bTest.testName(),
					bTest.printableBreif());

			if (bTest.didFail() == false) {
				mTestsSucceeded++;
			}

			if (mVerbosity.showTestResult()) {
				if (mVerbosity.showTestIntro()) {
					if (bTest.didFail()) {
						cout << "Test failed!" << endl;
					} else {
						if (mVerbosity.showTestResultSuccess()) {
							cout << "Test succeeded!" << endl;
						}
					}
				} else {
					if (bTest.didFail()) {
						cout << "Test failed: " << printable << endl;
					} else {
						if (mVerbosity.showTestResultSuccess()) {
							cout << "Test succeeded!" << endl;
						}
					}
				}
			}
		}
	}

	template<class TestWithParam, typename TypeParam>
	void execWithP(TypeParam const &aParam) {
		mTests++;
		auto bTest = TestWithParam(aParam);

		if (mVerbosity.showTestIntro()) {
			cout << "Running " << bTest.testName() << " with "
					<< aParam.printableName() << " ..." << endl;
		}

		TestRunnerInterface::setShowAssertionMsg(bTest,
				mVerbosity.showAssertReason());

		bTest.setup();

		try {
			bTest.run();
		} catch (AssertException &exc) {
			//Should we show the assert here?
		}

		bTest.teardown();

		bool repSts = true;
		if (mExecuted[bTest.testName()].valid == true) {
			repSts = mExecuted[bTest.testName()].status;
		}
		repSts &= !bTest.didFail();
		mExecuted[bTest.testName()] = TestStatus(repSts, bTest.testName(),
				bTest.printableBreif());

		if (bTest.didFail() == false) {
			mTestsSucceeded++;
		}

		if (mVerbosity.showTestResult()) {
			if (mVerbosity.showTestIntro()) {
				if (bTest.didFail()) {
					cout << "Test failed!" << endl;
				} else {
					if (mVerbosity.showTestResultSuccess()) {
						cout << "Test succeeded!" << endl;
					}
				}
			} else {
				if (bTest.didFail()) {
					cout << "Test failed: " << bTest.testName() << " with "
							<< aParam.printableName() << "!" << endl;
				} else {
					if (mVerbosity.showTestResultSuccess()) {
						cout << "Test succeeded!" << endl;
					}
				}
			}
		}
	}

	template<class TestWithParam, typename TypeParam>
	void execWithParam(Generator<TypeParam> const &aGen) {
		for (auto cParam = aGen.cbegin(); cParam < aGen.cend(); cParam++) {
			mTests++;
			auto bTest = TestWithParam(*cParam);

			if (mVerbosity.showTestIntro()) {
				cout << "Running " << bTest.testName() << " with "
						<< (*cParam).printableName() << " ..." << endl;
			}

			TestRunnerInterface::setShowAssertionMsg(bTest,
					mVerbosity.showAssertReason());

			bTest.setup();

			try {
				bTest.run();
			} catch (AssertException &exc) {
				//Should we show the assert here?
			}

			bTest.teardown();

			bool repSts = true;
			if (mExecuted[bTest.testName()].valid == true) {
				repSts = mExecuted[bTest.testName()].status;
			}
			repSts &= !bTest.didFail();
			mExecuted[bTest.testName()] = TestStatus(repSts, bTest.testName(),
					bTest.printableBreif());

			if (bTest.didFail() == false) {
				mTestsSucceeded++;
			}

			if (mVerbosity.showTestResult()) {
				if (mVerbosity.showTestIntro()) {
					if (bTest.didFail()) {
						cout << "Test failed!" << endl;
					} else {
						if (mVerbosity.showTestResultSuccess()) {
							cout << "Test succeeded!" << endl;
						}
					}
				} else {
					if (bTest.didFail()) {
						cout << "Test failed: " << bTest.testName() << " with "
								<< (*cParam).printableName() << "!" << endl;
					} else {
						if (mVerbosity.showTestResultSuccess()) {
							cout << "Test succeeded!" << endl;
						}
					}
				}
			}
		}
	}

	void printSummary(void) {
		cout << endl << endl;
		cout << "Summary:" << endl;
		cout << "======= " << endl;
		cout << "Tests executed  : " << mTests << endl;
		uint64_t testsFailed = mTests - mTestsSucceeded;
		if (testsFailed != 0) {
			cout << "Tests succeeded : " << mTestsSucceeded << endl;
			cout << "Tests failed    : " << testsFailed << endl;
		} else {
			cout << "Hurray! All tests succeeded!" << endl;
		}
	}

	void writeReport(string filename) {
		ofstream myfile;
		myfile.open(filename);
		for (auto pair : mExecuted) {
			TestStatus sts = pair.second;
			if(!sts.valid) {
				continue;
			}
			myfile << '"' << sts.name << '"' << ',' << (sts.status? "Pass": "Fail") << ',' << '"' << sts.breif << '"' << endl;
		}
		myfile.close();
	}
};

template<typename Type>
class GeneratorTypedParamUint8: public Generator<Type> {
	Generator<uint8_t> const &mGen;

public:
	explicit GeneratorTypedParamUint8(uint8_t aBegin, uint8_t aEnd) :
			mGen(*new GeneratorRange<uint8_t>(aBegin, aEnd)) {
	}

	explicit GeneratorTypedParamUint8(Generator<uint8_t> const &aSafeQ) :
			mGen(aSafeQ.clone()) {
	}

	explicit GeneratorTypedParamUint8(GeneratorTypedParamUint8 const & aOther) :
			mGen(aOther.mGen.clone()) {
	}

	uint_t size() const {
		return mGen.size();
	}

	Type at(uint_t const aIdx) const {
		return Type(mGen.at(aIdx));
	}

	GeneratorTypedParamUint8<Type>& clone() const {
		GeneratorTypedParamUint8<Type> *lRet = new GeneratorTypedParamUint8(mGen);

		return *lRet;
	}

	virtual ~GeneratorTypedParamUint8() {
		delete &mGen;
	}
};

#endif /* NOSH_NOSH_HPP_ */
