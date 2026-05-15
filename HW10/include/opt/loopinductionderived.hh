#ifndef __LOOPINDUCTIONDERIVED_HH__
#define __LOOPINDUCTIONDERIVED_HH__

#include "loopinductionbasic.hh"
#include <iostream>

struct AffineIVExpr {
	int basicTempNum;
	int basicCoeffConst;
	int basicCoeffInvTempNum;
	int constant;
	int invOffsetTempNum;
	int invOffsetScale;
	bool invOffsetNegative;
};

class DerivedInductionVar {
public:
	int headerLabel;
	int tempNum;
	int sourceTempNum;
	size_t sourceOrder;
	AffineIVExpr expr;
	quad::QuadStm* defStm;

	DerivedInductionVar(
		int headerLabel,
		int tempNum,
		int sourceTempNum,
		size_t sourceOrder,
		const AffineIVExpr& expr,
		quad::QuadStm* defStm
	) : headerLabel(headerLabel),
		tempNum(tempNum),
		sourceTempNum(sourceTempNum),
		sourceOrder(sourceOrder),
		expr(expr),
		defStm(defStm) {}
	
	void print() const {
		auto printSignedConst = [](int value, bool& printedAny) {
			if (value == 0) {
				return;
			}
			if (!printedAny) {
				std::cout << value;
				printedAny = true;
				return;
			}
			if (value > 0) {
				std::cout << " + " << value;
			} else {
				std::cout << " - " << (-value);
			}
		};

		auto printSignedTemp = [](int tempNum, bool negative, int scale, bool& printedAny) {
			if (tempNum == -1) {
				return;
			}
			if (!printedAny) {
				if (negative) {
					std::cout << "-";
				}
			} else {
				std::cout << (negative ? " - " : " + ");
			}
			if (scale != 1) {
				std::cout << scale << "*";
			}
			std::cout << "t" << tempNum;
			printedAny = true;
		};

		std::cout << "  t" << tempNum << " (derived IV)" << std::endl;
		std::cout << "    Depends on: t" << sourceTempNum << std::endl;
		std::cout << "    Source order: ";
		if (sourceOrder != static_cast<size_t>(-1)) {
			std::cout << sourceOrder << std::endl;
		} else {
			std::cout << "unknown" << std::endl;
		}
		std::cout << "    Expression: ";

		bool printedExpr = false;
		if (expr.basicCoeffConst != 0) {
			if (expr.basicCoeffConst < 0) {
				std::cout << "-";
			}
			int absCoeff = expr.basicCoeffConst >= 0
				? expr.basicCoeffConst
				: -expr.basicCoeffConst;
			if (absCoeff != 1) {
				std::cout << absCoeff << "*";
			}
			if (expr.basicCoeffInvTempNum != -1) {
				std::cout << "t" << expr.basicCoeffInvTempNum << "*";
			}
			std::cout << "t" << expr.basicTempNum;
			printedExpr = true;
		}

		printSignedTemp(
			expr.invOffsetTempNum,
			expr.invOffsetNegative,
			expr.invOffsetScale,
			printedExpr
		);
		printSignedConst(expr.constant, printedExpr);

		if (!printedExpr) {
			std::cout << "0";
		}
		std::cout << std::endl;
	}
};

map<int, vector<DerivedInductionVar>> discoverDerivedInductionVars(quad::QuadFuncDecl* func, LoopHeaderMap *loopHeaderMap, const DefUseChain& du, const ControlFlowInfo& cfi);

#endif
