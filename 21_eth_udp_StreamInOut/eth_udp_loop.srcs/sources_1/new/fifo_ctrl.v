`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/07/31 13:29:12
// Design Name: 
// Module Name: fifo_ctrl
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module fifo_ctrl(
    input clk,
    input rst_n,

    input rec_fifo_wren,
    input [31:0] rec_fifo_din,
    input rec_pkt_done,
    input [15:0] rec_byte_num,

    input tx_fifo_rden,
    output [31:0] tx_fifo_dout,
    input tx_pkt_done,
    output reg tx_start_en,
    output reg [15:0] tx_byte_num
    );


//定时累加器，硬件上电后即一直开始工作
reg [63:0] timerCnt;
always @(posedge clk, negedge rst_n)begin
	if(!rst_n)begin 
		timerCnt <= 64'd0;
	end 
	else begin
		timerCnt <= timerCnt + 64'd1;
	end	
end


reg [63:0] tPcToFpgaEnd;		//FPGA读取PC下发数据结束
reg [63:0] tFpgaToPcStart;      //FPGA写入CP上传数据开始
reg [63:0] tFpgaToPcEnd;		//FPGA写入CP上传数据结束
reg catch;							//计算各个时间段的节点
reg [2:0] state;
reg [15:0] num;


always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
        tPcToFpgaEnd <= 0;
        tFpgaToPcStart <= 0;
        tFpgaToPcEnd <= 0;
        catch <= 1'b0;
        state <= 0;
        num <= 0;
        fpga2pc_fifo_wren <= 0;
        pc2fpga_fifo_rden <= 0;
    end
    else begin
        case(state)
        0: begin
            catch <= 1'b0;
            if (rec_pkt_done) begin
                tPcToFpgaEnd <= timerCnt;
                pc2fpga_fifo_rden <= 1; //提前一个时钟开始
                state <= state + 1;
            end
        end

        1: begin
            state <= state + 1;
        end

        2: begin
            if (num >= rec_byte_num/4) begin
                num <= 0;
                fpga2pc_fifo_wren <= 0;
                state <= state + 1;
            end
            else begin
                num <= num + 1'b1;
                fpga2pc_fifo_wren <= 1;
                
                if (num == 1) begin
                    fpga2pc_fifo_din <= 32'h12_34_56_78;		//大小端测试，实际在PC端会是buf[4]=0x12
                                                                //                        buf[5]=0x34
                                                                //                        buf[6]=0x56
                                                                //                        buf[7]=0x78
                end
				else if (num == 32'd2) begin    
					fpga2pc_fifo_din = tPcLoop[63:32];      
				end
				else if (num == 32'd3) begin
					fpga2pc_fifo_din = tPcLoop[31:0];
				end 
				else begin
					fpga2pc_fifo_din <= pc2fpga_fifo_dout;
				end

                if(num >= rec_byte_num/4-2) 
                    pc2fpga_fifo_rden <= 0; //提前一个时钟结束
                else 
                    pc2fpga_fifo_rden <= 1;
            end
        end

        3: begin
            tFpgaToPcStart <= timerCnt;
            tx_byte_num <= rec_byte_num;
            tx_start_en <= 1;
            state <= state + 1;
        end

        4: begin
            tx_start_en <= 0;
            if (tx_pkt_done) begin
                tFpgaToPcEnd <= timerCnt;
                catch <= 1'b1;
                state <= 0;
            end
        end

        default begin
            state <= 0;
        end

        endcase
    end
end

reg [63:0] tCurPcToFpgaEnd;
reg [63:0] tCurFpgaToPcStart;
reg [63:0] tCurFpgaToPcEnd;
reg [63:0] tLastFpgaToPcEnd;
(* MARK_DEBUG="true" *) reg [63:0] tPcLoop;
(* MARK_DEBUG="true" *) reg [63:0] tFpgaRead;
(* MARK_DEBUG="true" *) reg [63:0] tFpgaWrite;
(* MARK_DEBUG="true" *) reg [63:0] tMaxPcLoop;
reg [2:0] tstate;
always @(posedge clk or negedge rst_n) begin
    if(!rst_n) begin
		tCurPcToFpgaEnd <= 64'd0;
        tCurFpgaToPcStart <= 64'd0;
		tCurFpgaToPcEnd <= 64'd0;
        tLastFpgaToPcEnd <= 64'd0;
        tPcLoop <= 64'd0;
        tFpgaRead <= 64'd0;
        tFpgaWrite <= 64'd0;
        tMaxPcLoop <= 64'd0;
        tstate <= 3'd0;
    end
    else begin
		case(tstate)
		0: begin
			//在catch时刻暂存所有时间数据
			if (catch==1'b1) begin
				tCurPcToFpgaEnd <= tPcToFpgaEnd;
                tCurFpgaToPcStart <= tFpgaToPcStart;
				tCurFpgaToPcEnd <= tFpgaToPcEnd;
				tstate <= tstate + 1'b1;
			end
		end

			//计算时间长度
		1: begin
            tFpgaRead <= tCurFpgaToPcStart - tCurPcToFpgaEnd;
			tFpgaWrite <= tCurFpgaToPcEnd - tCurFpgaToPcStart;
			tPcLoop <= tCurPcToFpgaEnd - tLastFpgaToPcEnd;       //将FPGA解析UDP包的时间包含在tPcLoop中了
			tstate <= tstate + 1'b1;
		end

			//暂存为前次时间数据，留作后期使用
		2: begin
			if (tMaxPcLoop > 64'd100_000_000) begin			//如果得到的值大于1S即为无效值
				tMaxPcLoop <= 64'd0;						//原因是定时器开机即启动且中间无停止
			end 
			else if (tMaxPcLoop < tPcLoop) begin
				tMaxPcLoop <= tPcLoop;
			end

			tLastFpgaToPcEnd <= tCurFpgaToPcEnd;
			tstate <= 0;
		end

		default: begin
			tstate <= 0;
		end

		endcase

    end
end


reg pc2fpga_fifo_rden;
wire [31 : 0] pc2fpga_fifo_dout;
reg fpga2pc_fifo_wren;
reg [31 : 0] fpga2pc_fifo_din;

//同步FIFO
sync_fifo_2048x32b u_pc2fpga_sync_fifo_2048x32b (
    .clk      (clk),  // input wire clk
    .rst      (~rst_n),  // input wire rst
    .din      (rec_fifo_din  ),  // input wire [31 : 0] din
    .wr_en    (rec_fifo_wren    ),  // input wire wr_en
    .rd_en    (pc2fpga_fifo_rden    ),  // input wire rd_en
    .dout     (pc2fpga_fifo_dout   ),  // output wire [31 : 0] dout
    .full     (),            // output wire full
    .empty    ()             // output wire empty
    ); 

//同步FIFO
sync_fifo_2048x32b u_fpga2pc_sync_fifo_2048x32b (
    .clk      (clk),  // input wire clk
    .rst      (~rst_n),  // input wire rst
    .din      (fpga2pc_fifo_din  ),  // input wire [31 : 0] din
    .wr_en    (fpga2pc_fifo_wren    ),  // input wire wr_en
    .rd_en    (tx_fifo_rden    ),  // input wire rd_en
    .dout     (tx_fifo_dout   ),  // output wire [31 : 0] dout
    .full     (),            // output wire full
    .empty    ()             // output wire empty
    ); 
endmodule
