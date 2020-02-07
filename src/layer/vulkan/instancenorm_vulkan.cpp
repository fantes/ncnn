// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include "instancenorm_vulkan.h"
#include <algorithm>

namespace ncnn {

DEFINE_LAYER_CREATOR(InstanceNorm_vulkan)

InstanceNorm_vulkan::InstanceNorm_vulkan()
{
    support_vulkan = true;

    pipeline_instancenorm_reduce_sum4_fp16_to_fp32 = 0;
    pipeline_instancenorm_reduce_sum4_fp32[0] = 0;
    pipeline_instancenorm_reduce_sum4_fp32[1] = 0;
    pipeline_instancenorm_reduce_mean = 0;
    pipeline_instancenorm_sub_mean_square = 0;
    pipeline_instancenorm_coeffs = 0;
    pipeline_instancenorm_norm = 0;

    pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4 = 0;
    pipeline_instancenorm_reduce_sum4_fp32_pack4[0] = 0;
    pipeline_instancenorm_reduce_sum4_fp32_pack4[1] = 0;
    pipeline_instancenorm_reduce_mean_pack4 = 0;
    pipeline_instancenorm_sub_mean_square_pack4 = 0;
    pipeline_instancenorm_coeffs_pack4 = 0;
    pipeline_instancenorm_norm_pack4 = 0;

    pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8 = 0;
    pipeline_instancenorm_reduce_sum4_fp32_pack8[0] = 0;
    pipeline_instancenorm_reduce_sum4_fp32_pack8[1] = 0;
    pipeline_instancenorm_reduce_mean_pack8 = 0;
    pipeline_instancenorm_sub_mean_square_pack8 = 0;
    pipeline_instancenorm_coeffs_pack8 = 0;
    pipeline_instancenorm_norm_pack8 = 0;
}

int InstanceNorm_vulkan::create_pipeline(const Option& opt)
{
    int elempack = opt.use_shader_pack8 && channels % 8 == 0 ? 8 : channels % 4 == 0 ? 4 : 1;

    std::vector<vk_specialization_type> specializations(1);
    specializations[0].f = eps;

    // pack1
    if (elempack == 1)
    {
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32 = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32->set_optimal_local_size_xyz(16, 1, channels);
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32->create("instancenorm_reduce_sum4_fp16_to_fp32", opt, std::vector<vk_specialization_type>(), 2, 6);

        pipeline_instancenorm_reduce_sum4_fp32[0] = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp32[0]->set_optimal_local_size_xyz(16, 1, channels);
        pipeline_instancenorm_reduce_sum4_fp32[0]->create("instancenorm_reduce_sum4_fp32", opt, std::vector<vk_specialization_type>(), 2, 6);
        pipeline_instancenorm_reduce_sum4_fp32[1] = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp32[1]->set_optimal_local_size_xyz(16, 1, channels);
        pipeline_instancenorm_reduce_sum4_fp32[1]->create("instancenorm_reduce_sum4_fp32", opt, std::vector<vk_specialization_type>(), 2, 6);

        pipeline_instancenorm_reduce_mean = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_mean->set_optimal_local_size_xyz(channels, 1, 1);
        pipeline_instancenorm_reduce_mean->create("instancenorm_reduce_mean", opt, std::vector<vk_specialization_type>(), 2, 4);

        pipeline_instancenorm_sub_mean_square = new Pipeline(vkdev);
        pipeline_instancenorm_sub_mean_square->set_optimal_local_size_xyz(32, 32, channels);
        pipeline_instancenorm_sub_mean_square->create("instancenorm_sub_mean_square", opt, std::vector<vk_specialization_type>(), 3, 5);

        pipeline_instancenorm_coeffs = new Pipeline(vkdev);
        pipeline_instancenorm_coeffs->set_optimal_local_size_xyz(channels, 1, 1);
        pipeline_instancenorm_coeffs->create("instancenorm_coeffs", opt, specializations, 5, 1);

        pipeline_instancenorm_norm = new Pipeline(vkdev);
        pipeline_instancenorm_norm->set_optimal_local_size_xyz(32, 32, channels);
        pipeline_instancenorm_norm->create("instancenorm_norm", opt, std::vector<vk_specialization_type>(), 2, 5);
    }

    // pack4
    if (elempack == 4)
    {
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4 = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4->set_optimal_local_size_xyz(16, 1, std::max(1, channels / 4));
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4->create("instancenorm_reduce_sum4_fp16_to_fp32_pack4", opt, std::vector<vk_specialization_type>(), 2, 6);

        pipeline_instancenorm_reduce_sum4_fp32_pack4[0] = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp32_pack4[0]->set_optimal_local_size_xyz(16, 1, std::max(1, channels / 4));
        pipeline_instancenorm_reduce_sum4_fp32_pack4[0]->create("instancenorm_reduce_sum4_fp32_pack4", opt, std::vector<vk_specialization_type>(), 2, 6);
        pipeline_instancenorm_reduce_sum4_fp32_pack4[1] = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp32_pack4[1]->set_optimal_local_size_xyz(16, 1, std::max(1, channels / 4));
        pipeline_instancenorm_reduce_sum4_fp32_pack4[1]->create("instancenorm_reduce_sum4_fp32_pack4", opt, std::vector<vk_specialization_type>(), 2, 6);

        pipeline_instancenorm_reduce_mean_pack4 = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_mean_pack4->set_optimal_local_size_xyz(std::max(1, channels / 4), 1, 1);
        pipeline_instancenorm_reduce_mean_pack4->create("instancenorm_reduce_mean_pack4", opt, std::vector<vk_specialization_type>(), 2, 4);

        pipeline_instancenorm_sub_mean_square_pack4 = new Pipeline(vkdev);
        pipeline_instancenorm_sub_mean_square_pack4->set_optimal_local_size_xyz(32, 32, std::max(1, channels / 4));
        pipeline_instancenorm_sub_mean_square_pack4->create("instancenorm_sub_mean_square_pack4", opt, std::vector<vk_specialization_type>(), 3, 5);

        pipeline_instancenorm_coeffs_pack4 = new Pipeline(vkdev);
        pipeline_instancenorm_coeffs_pack4->set_optimal_local_size_xyz(std::max(1, channels / 4), 1, 1);
        pipeline_instancenorm_coeffs_pack4->create("instancenorm_coeffs_pack4", opt, specializations, 5, 1);

        pipeline_instancenorm_norm_pack4 = new Pipeline(vkdev);
        pipeline_instancenorm_norm_pack4->set_optimal_local_size_xyz(32, 32, std::max(1, channels / 4));
        pipeline_instancenorm_norm_pack4->create("instancenorm_norm_pack4", opt, std::vector<vk_specialization_type>(), 2, 5);
    }

    // pack8
    if (elempack == 8)
    {
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8 = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8->set_optimal_local_size_xyz(16, 1, std::max(1, channels / 8));
        pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8->create("instancenorm_reduce_sum4_fp16_to_fp32_pack8", opt, std::vector<vk_specialization_type>(), 2, 6);

        pipeline_instancenorm_reduce_sum4_fp32_pack8[0] = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp32_pack8[0]->set_optimal_local_size_xyz(16, 1, std::max(1, channels / 8));
        pipeline_instancenorm_reduce_sum4_fp32_pack8[0]->create("instancenorm_reduce_sum4_fp32_pack8", opt, std::vector<vk_specialization_type>(), 2, 6);
        pipeline_instancenorm_reduce_sum4_fp32_pack8[1] = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_sum4_fp32_pack8[1]->set_optimal_local_size_xyz(16, 1, std::max(1, channels / 8));
        pipeline_instancenorm_reduce_sum4_fp32_pack8[1]->create("instancenorm_reduce_sum4_fp32_pack8", opt, std::vector<vk_specialization_type>(), 2, 6);

        pipeline_instancenorm_reduce_mean_pack8 = new Pipeline(vkdev);
        pipeline_instancenorm_reduce_mean_pack8->set_optimal_local_size_xyz(std::max(1, channels / 8), 1, 1);
        pipeline_instancenorm_reduce_mean_pack8->create("instancenorm_reduce_mean_pack8", opt, std::vector<vk_specialization_type>(), 2, 4);

        pipeline_instancenorm_sub_mean_square_pack8 = new Pipeline(vkdev);
        pipeline_instancenorm_sub_mean_square_pack8->set_optimal_local_size_xyz(32, 32, std::max(1, channels / 8));
        pipeline_instancenorm_sub_mean_square_pack8->create("instancenorm_sub_mean_square_pack8", opt, std::vector<vk_specialization_type>(), 3, 5);

        pipeline_instancenorm_coeffs_pack8 = new Pipeline(vkdev);
        pipeline_instancenorm_coeffs_pack8->set_optimal_local_size_xyz(std::max(1, channels / 8), 1, 1);
        pipeline_instancenorm_coeffs_pack8->create("instancenorm_coeffs_pack8", opt, specializations, 5, 1);

        pipeline_instancenorm_norm_pack8 = new Pipeline(vkdev);
        pipeline_instancenorm_norm_pack8->set_optimal_local_size_xyz(32, 32, std::max(1, channels / 8));
        pipeline_instancenorm_norm_pack8->create("instancenorm_norm_pack8", opt, std::vector<vk_specialization_type>(), 2, 5);
    }

    return 0;
}

int InstanceNorm_vulkan::destroy_pipeline(const Option& /*opt*/)
{
    delete pipeline_instancenorm_reduce_sum4_fp16_to_fp32;
    pipeline_instancenorm_reduce_sum4_fp16_to_fp32 = 0;

    delete pipeline_instancenorm_reduce_sum4_fp32[0];
    delete pipeline_instancenorm_reduce_sum4_fp32[1];
    pipeline_instancenorm_reduce_sum4_fp32[0] = 0;
    pipeline_instancenorm_reduce_sum4_fp32[1] = 0;

    delete pipeline_instancenorm_reduce_mean;
    pipeline_instancenorm_reduce_mean = 0;

    delete pipeline_instancenorm_sub_mean_square;
    pipeline_instancenorm_sub_mean_square = 0;

    delete pipeline_instancenorm_coeffs;
    pipeline_instancenorm_coeffs = 0;

    delete pipeline_instancenorm_norm;
    pipeline_instancenorm_norm = 0;

    delete pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4;
    pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4 = 0;

    delete pipeline_instancenorm_reduce_sum4_fp32_pack4[0];
    delete pipeline_instancenorm_reduce_sum4_fp32_pack4[1];
    pipeline_instancenorm_reduce_sum4_fp32_pack4[0] = 0;
    pipeline_instancenorm_reduce_sum4_fp32_pack4[1] = 0;

    delete pipeline_instancenorm_reduce_mean_pack4;
    pipeline_instancenorm_reduce_mean_pack4 = 0;

    delete pipeline_instancenorm_sub_mean_square_pack4;
    pipeline_instancenorm_sub_mean_square_pack4 = 0;

    delete pipeline_instancenorm_coeffs_pack4;
    pipeline_instancenorm_coeffs_pack4 = 0;

    delete pipeline_instancenorm_norm_pack4;
    pipeline_instancenorm_norm_pack4 = 0;

    delete pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8;
    pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8 = 0;

    delete pipeline_instancenorm_reduce_sum4_fp32_pack8[0];
    delete pipeline_instancenorm_reduce_sum4_fp32_pack8[1];
    pipeline_instancenorm_reduce_sum4_fp32_pack8[0] = 0;
    pipeline_instancenorm_reduce_sum4_fp32_pack8[1] = 0;

    delete pipeline_instancenorm_reduce_mean_pack8;
    pipeline_instancenorm_reduce_mean_pack8 = 0;

    delete pipeline_instancenorm_sub_mean_square_pack8;
    pipeline_instancenorm_sub_mean_square_pack8 = 0;

    delete pipeline_instancenorm_coeffs_pack8;
    pipeline_instancenorm_coeffs_pack8 = 0;

    delete pipeline_instancenorm_norm_pack8;
    pipeline_instancenorm_norm_pack8 = 0;

    return 0;
}

int InstanceNorm_vulkan::upload_model(VkTransfer& cmd, const Option& opt)
{
    cmd.record_upload(gamma_data, gamma_data_gpu, opt);

    cmd.record_upload(beta_data, beta_data_gpu, opt);

    return 0;
}

int InstanceNorm_vulkan::forward_inplace(VkMat& bottom_top_blob, VkCompute& cmd, const Option& opt) const
{
    int w = bottom_top_blob.w;
    int h = bottom_top_blob.h;
    int size = w * h;
    size_t elemsize = bottom_top_blob.elemsize;
    int elempack = bottom_top_blob.elempack;

    // mean
    VkMat mean_workspace(channels, elemsize, elempack, opt.workspace_vkallocator, opt.staging_vkallocator);
    {
        // reduce sum
        VkMat sum_workspace;
        {
        int reduced_w = (bottom_top_blob.w * bottom_top_blob.h + 3) / 4;
        int reduced_h = 1;
        int reduced_c = bottom_top_blob.c;

        sum_workspace.create(reduced_w, reduced_h, reduced_c, 4u*elempack, elempack, opt.workspace_vkallocator, opt.staging_vkallocator);
        {
        std::vector<VkMat> bindings(2);
        bindings[0] = bottom_top_blob;
        bindings[1] = sum_workspace;

        std::vector<vk_constant_type> constants(6);
        constants[0].i = bottom_top_blob.w * bottom_top_blob.h;
        constants[1].i = bottom_top_blob.c;
        constants[2].i = bottom_top_blob.cstep;
        constants[3].i = sum_workspace.w;
        constants[4].i = sum_workspace.c;
        constants[5].i = sum_workspace.cstep;

        const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack8
                                 : elempack == 4 ? pipeline_instancenorm_reduce_sum4_fp16_to_fp32_pack4
                                 : pipeline_instancenorm_reduce_sum4_fp16_to_fp32;

        cmd.record_pipeline(pipeline, bindings, constants, sum_workspace);
        }
        }

        int pb = 0;
        while (sum_workspace.w > 4)
        {
        int reduced_w = (sum_workspace.w + 3) / 4;
        int reduced_h = 1;
        int reduced_c = sum_workspace.c;

        VkMat sum_workspace_reduced;
        sum_workspace_reduced.create(reduced_w, reduced_h, reduced_c, 4u*elempack, elempack, opt.workspace_vkallocator, opt.staging_vkallocator);

        {
        std::vector<VkMat> bindings(2);
        bindings[0] = sum_workspace;
        bindings[1] = sum_workspace_reduced;

        std::vector<vk_constant_type> constants(6);
        constants[0].i = sum_workspace.w;
        constants[1].i = sum_workspace.c;
        constants[2].i = sum_workspace.cstep;
        constants[3].i = sum_workspace_reduced.w;
        constants[4].i = sum_workspace_reduced.c;
        constants[5].i = sum_workspace_reduced.cstep;

        const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_reduce_sum4_fp32_pack8[pb%2]
                                 : elempack == 4 ? pipeline_instancenorm_reduce_sum4_fp32_pack4[pb%2]
                                 : pipeline_instancenorm_reduce_sum4_fp32[pb%2];

        cmd.record_pipeline(pipeline, bindings, constants, sum_workspace_reduced);

        pb++;
        }

        sum_workspace = sum_workspace_reduced;
        }

        {
        std::vector<VkMat> bindings(2);
        bindings[0] = sum_workspace;
        bindings[1] = mean_workspace;

        std::vector<vk_constant_type> constants(4);
        constants[0].i = sum_workspace.w;
        constants[1].i = sum_workspace.c;
        constants[2].i = sum_workspace.cstep;
        constants[3].f = size;

        const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_reduce_mean_pack8
                                 : elempack == 4 ? pipeline_instancenorm_reduce_mean_pack4
                                 : pipeline_instancenorm_reduce_mean;

        cmd.record_pipeline(pipeline, bindings, constants, mean_workspace);
        }
    }

    // var
    VkMat var_workspace(channels, elemsize, elempack, opt.workspace_vkallocator, opt.staging_vkallocator);
    {
        // sub mean and square
        VkMat square_workspace;
        square_workspace.create(w, h, channels, 4u*elempack, elempack, opt.workspace_vkallocator, opt.staging_vkallocator);
        {
        std::vector<VkMat> bindings(3);
        bindings[0] = bottom_top_blob;
        bindings[1] = mean_workspace;
        bindings[2] = square_workspace;

        std::vector<vk_constant_type> constants(5);
        constants[0].i = bottom_top_blob.dims;
        constants[1].i = bottom_top_blob.w;
        constants[2].i = bottom_top_blob.h;
        constants[3].i = bottom_top_blob.c;
        constants[4].i = bottom_top_blob.cstep;

        const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_sub_mean_square_pack8
                                 : elempack == 4 ? pipeline_instancenorm_sub_mean_square_pack4
                                 : pipeline_instancenorm_sub_mean_square;

        cmd.record_pipeline(pipeline, bindings, constants, square_workspace);
        }

        // reduce square
        VkMat sqsum_workspace = square_workspace;
        sqsum_workspace.w = sqsum_workspace.w * sqsum_workspace.h;
        sqsum_workspace.h = 1;

        int pb = 0;
        while (sqsum_workspace.w > 4)
        {
        int reduced_w = (sqsum_workspace.w + 3) / 4;
        int reduced_h = 1;
        int reduced_c = sqsum_workspace.c;

        VkMat sqsum_workspace_reduced;
        sqsum_workspace_reduced.create(reduced_w, reduced_h, reduced_c, 4u*elempack, elempack, opt.workspace_vkallocator, opt.staging_vkallocator);

        {
        std::vector<VkMat> bindings(2);
        bindings[0] = sqsum_workspace;
        bindings[1] = sqsum_workspace_reduced;

        std::vector<vk_constant_type> constants(6);
        constants[0].i = sqsum_workspace.w;
        constants[1].i = sqsum_workspace.c;
        constants[2].i = sqsum_workspace.cstep;
        constants[3].i = sqsum_workspace_reduced.w;
        constants[4].i = sqsum_workspace_reduced.c;
        constants[5].i = sqsum_workspace_reduced.cstep;

        const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_reduce_sum4_fp32_pack8[pb%2]
                                 : elempack == 4 ? pipeline_instancenorm_reduce_sum4_fp32_pack4[pb%2]
                                 : pipeline_instancenorm_reduce_sum4_fp32[pb%2];

        cmd.record_pipeline(pipeline, bindings, constants, sqsum_workspace_reduced);

        pb++;
        }

        sqsum_workspace = sqsum_workspace_reduced;
        }

        {
        std::vector<VkMat> bindings(2);
        bindings[0] = sqsum_workspace;
        bindings[1] = var_workspace;

        std::vector<vk_constant_type> constants(4);
        constants[0].i = sqsum_workspace.w;
        constants[1].i = sqsum_workspace.c;
        constants[2].i = sqsum_workspace.cstep;
        constants[3].f = size;

        const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_reduce_mean_pack8
                                 : elempack == 4 ? pipeline_instancenorm_reduce_mean_pack4
                                 : pipeline_instancenorm_reduce_mean;

        cmd.record_pipeline(pipeline, bindings, constants, var_workspace);
        }
    }

    // coeffs
    VkMat coeffs_workspace;
    coeffs_workspace.create(channels, elemsize * 2, elempack * 2, opt.workspace_vkallocator, opt.staging_vkallocator);
    {
    std::vector<VkMat> bindings(5);
    bindings[0] = coeffs_workspace;
    bindings[1] = mean_workspace;
    bindings[2] = var_workspace;
    bindings[3] = gamma_data_gpu;
    bindings[4] = beta_data_gpu;

    std::vector<vk_constant_type> constants(1);
    constants[0].i = channels;

    const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_coeffs_pack8
                             : elempack == 4 ? pipeline_instancenorm_coeffs_pack4
                             : pipeline_instancenorm_coeffs;

    cmd.record_pipeline(pipeline, bindings, constants, coeffs_workspace);
    }

    // norm
    {
    std::vector<VkMat> bindings(2);
    bindings[0] = bottom_top_blob;
    bindings[1] = coeffs_workspace;

    std::vector<vk_constant_type> constants(5);
    constants[0].i = bottom_top_blob.dims;
    constants[1].i = bottom_top_blob.w;
    constants[2].i = bottom_top_blob.h;
    constants[3].i = bottom_top_blob.c;
    constants[4].i = bottom_top_blob.cstep;

    const Pipeline* pipeline = elempack == 8 ? pipeline_instancenorm_norm_pack8
                             : elempack == 4 ? pipeline_instancenorm_norm_pack4
                             : pipeline_instancenorm_norm;

    cmd.record_pipeline(pipeline, bindings, constants, bottom_top_blob);
    }

    return 0;
}

} // namespace ncnn
