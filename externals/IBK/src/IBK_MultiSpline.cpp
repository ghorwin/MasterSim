#include "IBK_MultiSpline.h"

#include "IBK_assert.h"
#include "IBK_math.h"

namespace IBK {

void MultiSpline::set(const std::vector<double> baseValues, const std::vector<std::pair<std::vector<double>,std::vector<double>>>& values) {
	IBK_ASSERT(baseValues.size() == values.size());
	IBK_ASSERT(!baseValues.empty());

	m_baseValues = baseValues;
	for(const auto& lp : values) {
		m_values.push_back(IBK::LinearSpline());
		m_values.back().setValues(lp.first,lp.second);
	}
}

double MultiSpline::value(double x1, double x2) {
	std::pair<size_t,size_t> indices = indexArea(x1);
	if(indices.first == indices.second) {
		IBK_ASSERT(indices.first < m_values.size());
		return m_values[indices.first].value(x2);
	}
	double fact = (m_values[indices.second].value(x2) - m_values[indices.first].value(x2)) /
			(m_baseValues[indices.second] - m_baseValues[indices.first]);
	double res = m_values[indices.second].value(x2) - (m_baseValues[indices.second] - x1) * fact;
	return res;
}

std::pair<size_t,size_t> MultiSpline::indexArea(double x1) {
	if(m_baseValues.empty() || x1 < m_baseValues.front() || IBK::near_zero(x1))
		return {0,0};
	if(x1 > m_baseValues.back() || IBK::near_equal(m_baseValues.back(),x1))
		return {m_baseValues.size()-1,m_baseValues.size()-1};
	for(size_t i=0; i<m_baseValues.size(); ++i) {
		if(IBK::near_equal(m_baseValues[i],x1))
			return {i,i};
	}
	size_t index = std::lower_bound(m_baseValues.begin(), m_baseValues.end(), x1)-m_baseValues.begin();
	return {index,index+1};
}


} // namespace IBK
