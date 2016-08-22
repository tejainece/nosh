#ifndef NOSH_NOSH_HPP_
#define NOSH_NOSH_HPP_

#include <iostream>
#include <iterator>
#include <vector>
#include <stdint.h>
#include <memory>
#include <map>

using namespace std;

typedef unsigned int uint_t;

template<typename Type>
class Generator;

template<typename Type>
class Iterator;

template<typename Type>
class Iterator: public std::iterator<std::input_iterator_tag, Type> {
	Generator<Type> &mGen;

	uint_t mIdx;
public:
	Iterator(Generator<Type> &aGen, uint_t aIdx) :
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

	Type operator*() {
		return mGen.at(mIdx);
	}
};

template<typename Type>
class Generator {
public:
	virtual Iterator<Type> begin() = 0;

	virtual Iterator<Type> end() = 0;

	virtual uint_t size() const = 0;

	virtual Type at(uint_t const aIdx) const = 0;

	virtual ~Generator() {

	}
};

template<typename Type>
class GeneratorList: public Generator<Type> {
private:
	vector<Type> const mVec;
public:
	GeneratorList(vector<Type> const aVec) :
			mVec(aVec) {
	}

	virtual Iterator<Type> begin() {
		return Iterator<Type>(*this, 0);
	}

	virtual Iterator<Type> end() {
		return Iterator<Type>(*this, mVec.size());
	}

	virtual uint_t size() const {
		return mVec.size();
	}

	virtual Type at(uint_t const aIdx) const {
		return mVec.at(aIdx);
	}

	virtual ~GeneratorList() {
	}
};

template<typename Type>
class RangeGenerator: public Generator<Type> {
private:
	Type const mStart;

	Type const mEnd;

	Type const mStep;
public:
	RangeGenerator(Type aStart, Type aEnd, Type aStep) :
			mStart(aStart), mEnd(aEnd), mStep(aStep) {
	}

	RangeGenerator(Type aStart, Type aEnd) :
			mStart(aStart), mEnd(aEnd), mStep(1) {
	}

	virtual ~RangeGenerator() {
	}

	Iterator<Type> begin() {
		return Iterator<Type>(*this, 0);
	}

	Iterator<Type> end() {
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
			return lPos;
		}

		return mEnd;
	}
};

template<typename Type>
RangeGenerator<Type> range(Type aStart, Type aEnd) {
	return RangeGenerator<Type>(aStart, aEnd, 1);
}

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
};

class Param {
public:
	virtual ~Param() {

	}

	virtual string printableName() = 0;
};

class LineInfo {
	char const * const mFilename;

	uint_t const mLineNum;
public:
	LineInfo(char const * const aFilename, uint_t const aLineNum) :
			mFilename(aFilename), mLineNum(aLineNum) {
	}

	char const * const filename(void) const {
		return mFilename;
	}

	uint_t lineNum(void) const {
		return mLineNum;
	}
};

#define kLineInfo LineInfo(__FILE__, __LINE__)

class Test {
	bool mTstFailed;
public:
	virtual ~Test() {
	}

	virtual void setup() = 0;

	virtual void teardown() = 0;

	virtual void run() = 0;

	template<typename Type>
	void expectEqual(Type const aLhs, Type const aRhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return;
		}

		mTstFailed = true;

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

	template<typename Type>
	void expectEqual(Type const aLhs, Type const aRhs,
			LineInfo const & aLine) {
		expectEqual(aLhs, aRhs, aLine, { });
	}

	void expectEqualUInt8(uint8_t const aLhs, uint8_t const aRhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return;
		}

		mTstFailed = true;

		cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
				<< " => Test case failed!" << endl;
		cout << "\t Reason => " << (uint16_t)aLhs << " != " << (uint16_t)aRhs << endl;
		if (!aContext.empty()) {
			cout << "\t Context => ";
			for (auto cContext : aContext) {
				cout << cContext.first << ": " << cContext.second << endl;
			}
		}
	}

	void expectEqualUInt8(uint8_t const aLhs, uint8_t const aRhs,
			LineInfo const & aLine) {
		expectEqualUInt8(aLhs, aRhs, aLine, { });
	}

	void expectEqualUInt16(uint16_t const aLhs, uint16_t const aRhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return;
		}

		mTstFailed = true;

		cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
				<< " => Test case failed!" << endl;
		cout << "\t Reason => " << (uint16_t)aLhs << " != " << (uint16_t)aRhs << endl;
		if (!aContext.empty()) {
			cout << "\t Context => ";
			for (auto cContext : aContext) {
				cout << cContext.first << ": " << cContext.second << endl;
			}
		}
	}

	void expectEqualUInt16(uint16_t const aLhs, uint16_t const aRhs,
			LineInfo const & aLine) {
		expectEqualUInt16(aLhs, aRhs, aLine, { });
	}

	void expectEqualUInt32(uint32_t const aLhs, uint32_t const aRhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == aRhs) {
			return;
		}

		mTstFailed = true;

		cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
				<< " => Test case failed!" << endl;
		cout << "\t Reason => " << (uint32_t)aLhs << " != " << (uint32_t)aRhs << endl;
		if (!aContext.empty()) {
			cout << "\t Context => ";
			for (auto cContext : aContext) {
				cout << cContext.first << ": " << cContext.second << endl;
			}
		}
	}

	void expectEqualUInt32(uint32_t const aLhs, uint32_t const aRhs,
			LineInfo const & aLine) {
		expectEqualUInt32(aLhs, aRhs, aLine, { });
	}

	void expectTrue(bool const aLhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs != false) {
			return;
		}

		mTstFailed = true;

		cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
				<< " => Test case failed!" << endl;
		cout << "\t Reason => " << (bool)aLhs << " not true!" << endl;
		if (!aContext.empty()) {
			cout << "\t Context => ";
			for (auto cContext : aContext) {
				cout << cContext.first << ": " << cContext.second << endl;
			}
		}
	}

	void expectTrue(bool const aLhs, LineInfo const &aLine) {
		expectTrue(aLhs, aLine, { });
	}

	void expectFalse(bool const aLhs, LineInfo const & aLine,
			map<string, string> const &aContext) {
		if (aLhs == false) {
			return;
		}

		mTstFailed = true;

		cout << aLine.filename() << ":" << (uint_t) aLine.lineNum()
				<< " => Test case failed!" << endl;
		cout << "\t Reason => " << (bool)aLhs << " not false!" << endl;
		if (!aContext.empty()) {
			cout << "\t Context => ";
			for (auto cContext : aContext) {
				cout << cContext.first << ": " << cContext.second << endl;
			}
		}
	}

	void expectFalse(bool const aLhs, LineInfo const &aLine) {
		expectFalse(aLhs, aLine, { });
	}
};

template<typename TypeParam>
class TestWithParam: public Test {
public:
	TestWithParam(TypeParam &aParam) {

	}

	virtual ~TestWithParam() {

	}
};

static void execTest(Test &aTest) {
	aTest.setup();

	aTest.run();

	aTest.teardown();
}

template<class TestWithParam, typename TypeParam>
static void execTestWithParam(Generator<TypeParam> &aGen) {
	for (auto cParam : aGen) {
		auto bTest = TestWithParam(cParam);
		execTest(bTest);
	}
}

#endif /* NOSH_NOSH_HPP_ */
