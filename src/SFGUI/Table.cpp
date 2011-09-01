#include <SFGUI/Table.hpp>

#include <set>

namespace sfg {

Table::Table() {
	OnSizeAllocate.Connect( &Table::HandleSizeAllocate, this );
}

Table::Ptr Table::Create() {
	return Ptr( new Table );
}

sf::Vector2f Table::GetRequisitionImpl() const {
	sf::Vector2f size( 0.f, 0.f );

	// Update requisitions.
	UpdateRequisitions();

	// Count allocations of columns.
	TableOptionsArray::const_iterator column_iter( m_columns.begin() );
	TableOptionsArray::const_iterator column_iter_end( m_columns.end() );

	for( ; column_iter != column_iter_end; ++column_iter ) {
		size.x += column_iter->requisition;
	}

	// Count allocations of rows.
	TableOptionsArray::const_iterator row_iter( m_rows.begin() );
	TableOptionsArray::const_iterator row_iter_end( m_rows.end() );

	for( ; row_iter != row_iter_end; ++row_iter ) {
		size.y += row_iter->requisition;
	}

	return size;
}

void Table::Attach( Widget::Ptr widget, sf::Uint32 left, sf::Uint32 right, sf::Uint32 top, sf::Uint32 bottom, int x_options, int y_options ) {
	assert( right > left );
	assert( bottom > top );

	// Store widget in a table cell object.
	m_cells.push_back( priv::TableCell( widget, sf::Rect<sf::Uint32>( left, top, right - left, bottom - top ), x_options, y_options ) );

	// Check if we need to enlarge rows/columns.
	if( left >= m_columns.size() ) {
		m_columns.resize( left + 1 );
	}

	if( top >= m_rows.size() ) {
		m_rows.resize( top + 1 );
	}

	// Add widget to container.
	Add( widget );

	// Request new size.
	RequestSize();
}


void Table::HandleSizeAllocate( Widget::Ptr widget, const sf::FloatRect& /*old_allocation*/ ) {
	AllocateChildrenSizes();
}

void Table::UpdateRequisitions() const {
	// Reset requisitions and expand flags, at first.
	for( std::size_t column_index = 0; column_index < m_columns.size(); ++column_index ) {
		m_columns[column_index].requisition = 0.f;
		m_columns[column_index].expand = false;
	}

	for( std::size_t row_index = 0; row_index < m_rows.size(); ++row_index ) {
		m_rows[row_index].requisition = 0.f;
		m_rows[row_index].expand = false;
	}

	// Iterate over children to get the maximum requisition for each column and
	// row and set expand flags.
	TableCellList::const_iterator cell_iter( m_cells.begin() );
	TableCellList::const_iterator cell_iter_end( m_cells.end() );

	for( ; cell_iter != cell_iter_end; ++cell_iter ) {
		m_columns[cell_iter->rect.Left].requisition = std::max( m_columns[cell_iter->rect.Left].requisition, cell_iter->child->GetRequisition().x );

		if( !m_columns[cell_iter->rect.Left].expand ) {
			m_columns[cell_iter->rect.Left].expand = (cell_iter->x_options & EXPAND);
		}

		m_rows[cell_iter->rect.Top].requisition = std::max( m_rows[cell_iter->rect.Top].requisition, cell_iter->child->GetRequisition().y );

		if( !m_rows[cell_iter->rect.Top].expand ) {
			m_rows[cell_iter->rect.Top].expand = (cell_iter->y_options & EXPAND);
		}
	}
}

void Table::AllocateChildrenSizes() {
	// Process columns.
	float width( 0.f );
	std::size_t num_expand( 0 );

	// Calculate "normal" width of columns and count expanded columns.
	for( std::size_t column_index = 0; column_index < m_columns.size(); ++column_index ) {
		// Every allocaction will be at least as wide as the requisition.
		m_columns[column_index].allocation = m_columns[column_index].requisition;

		// Calc position.
		if( column_index == 0 ) {
			m_columns[column_index].position = GetAllocation().Left;
		}
		else {
			m_columns[column_index].position = m_columns[column_index - 1].position + m_columns[column_index - 1].allocation;
		}

		// Count expanded columns.
		if( m_columns[column_index].expand ) {
			++num_expand;
		}

		width += m_columns[column_index].requisition;
	}

	// If there're expanded columns, we need to set the proper allocation for them.
	if( num_expand > 0 ) {
		float extra( (GetAllocation().Width - width) / static_cast<float>( num_expand ) );

		for( std::size_t column_index = 0; column_index < m_columns.size(); ++column_index ) {
			if( !m_columns[column_index].expand ) {
				continue;
			}

			m_columns[column_index].allocation += extra;
		}
	}

	// Process rows.
	float height( 0.f );
	num_expand = 0;

	// Calculate "normal" height of columns and count expanded rows.
	for( std::size_t row_index = 0; row_index < m_rows.size(); ++row_index ) {
		// Every allocaction will be at least as wide as the requisition.
		m_rows[row_index].allocation = m_rows[row_index].requisition;

		// Calc position.
		if( row_index == 0 ) {
			m_rows[row_index].position = GetAllocation().Top;
		}
		else {
			m_rows[row_index].position = m_rows[row_index - 1].position + m_rows[row_index - 1].allocation;
		}

		// Count expanded rows.
		if( m_rows[row_index].expand ) {
			++num_expand;
		}

		height += m_rows[row_index].requisition;
	}

	// If there're expanded rows, we need to set the proper allocation for them.
	if( num_expand > 0 ) {
		float extra( (GetAllocation().Height - height) / static_cast<float>( num_expand ) );

		for( std::size_t row_index = 0; row_index < m_rows.size(); ++row_index ) {
			if( !m_rows[row_index].expand ) {
				continue;
			}

			m_rows[row_index].allocation += extra;
		}
	}

	// Allocate children sizes.
	TableCellList::iterator cell_iter( m_cells.begin() );
	TableCellList::iterator cell_iter_end( m_cells.end() );

	for( ; cell_iter != cell_iter_end; ++cell_iter ) {
		cell_iter->child->AllocateSize(
			sf::FloatRect(
				m_columns[cell_iter->rect.Left].position,
				m_rows[cell_iter->rect.Top].position,
				m_columns[cell_iter->rect.Left].allocation,
				m_rows[cell_iter->rect.Top].allocation
			)
		);
	}
}

}