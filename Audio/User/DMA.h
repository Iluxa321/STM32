#ifndef __DMA__
#define __DMA__

#include "RTE_Components.h"
#include CMSIS_device_header

#define DMA_En(Channel) (Channel->CCR |= DMA_CCR_EN)
#define DMA_Dis(Channel) (Channel->CCR &= ~DMA_CCR_EN)


#define MEM2MEM_En 	((uint32_t)(0x1U << DMA_CCR_MEM2MEM_Pos))
#define MEM2MEM_Dis ((uint32_t)(0x0U << DMA_CCR_MEM2MEM_Pos))

#define PL_Low 			((uint32_t)(0x0U << DMA_CCR_PL_Pos))
#define PL_Medium 	((uint32_t)(0x1U << DMA_CCR_PL_Pos))	
#define PL_High 		((uint32_t)(0x2U << DMA_CCR_PL_Pos))
#define PL_VeryHigh ((uint32_t)(0x3U << DMA_CCR_PL_Pos))

#define MSIZE_8 		((uint32_t)(0x0U << DMA_CCR_MSIZE_Pos))
#define MSIZE_16 		((uint32_t)(0x1U << DMA_CCR_MSIZE_Pos))
#define MSIZE_32 		((uint32_t)(0x2U << DMA_CCR_MSIZE_Pos))

#define PSIZE_8 		((uint32_t)(0x0U << DMA_CCR_PSIZE_Pos))
#define PSIZE_16 		((uint32_t)(0x1U << DMA_CCR_PSIZE_Pos))
#define PSIZE_32 		((uint32_t)(0x2U << DMA_CCR_PSIZE_Pos))

#define MINC_En 		((uint32_t)(0x1U << DMA_CCR_MINC_Pos))
#define MINC_Dis 		((uint32_t)(0x0U << DMA_CCR_MINC_Pos))

#define PINC_En 		((uint32_t)(0x1U << DMA_CCR_PINC_Pos))
#define PINC_Dis 		((uint32_t)(0x0U << DMA_CCR_PINC_Pos))

#define CIRC_En 		((uint32_t)(0x1U << DMA_CCR_CIRC_Pos))
#define CIRC_Dis 		((uint32_t)(0x0U << DMA_CCR_CIRC_Pos))

#define MEM2PER 		((uint32_t)(0x1U << DMA_CCR_DIR_Pos))
#define PER2MEM 		((uint32_t)(0x0U << DMA_CCR_DIR_Pos))
	
#define TEIE_En		 	((uint32_t)(0x1U << DMA_CCR_TEIE_Pos))
#define TEIE_Dis	 	((uint32_t)(0x0U << DMA_CCR_TEIE_Pos))

#define HTIE_En 	 	((uint32_t)(0x1U << DMA_CCR_HTIE_Pos))
#define HTIE_Dis 	 	((uint32_t)(0x0U << DMA_CCR_HTIE_Pos))

#define TCIE_En 		((uint32_t)(0x1U << DMA_CCR_TCIE_Pos))
#define TCIE_Dis 		((uint32_t)(0x0U << DMA_CCR_TCIE_Pos))


void DMA_DeInit(DMA_Channel_TypeDef *Channel);
void DMA_Init(DMA_Channel_TypeDef *Channel, uint32_t perif, uint32_t mem, uint32_t cnt, uint32_t config);



#endif